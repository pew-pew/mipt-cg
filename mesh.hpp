#pragma once

#include <cassert>
#include <cstdio>
#include <fstream>
#include <vector>

#include <glm/glm.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <GL/glew.h>

struct Mesh {
  /**
   * VAO attributes:
   * 0 - vec3 position
   * 1 - vec3 color
   * 2 - vec2 texture coords
   * 3 - vec3 normals
   */

  struct Vertex {
    glm::vec3 pos;
    glm::vec3 color;
    glm::vec2 tex_coord;
    glm::vec3 normal;
  };
  static_assert(sizeof(Vertex) == sizeof(float) * 11);

  std::vector<Vertex> vertices;
  std::vector<uint32_t> indices; // not used wisely
  uint vbo = 0;
  uint vao = 0;
  uint ebo = 0;

  Mesh(const Mesh&) = delete;
  Mesh& operator=(const Mesh&) = delete;

  ~Mesh() {
    glDeleteBuffers(1, &ebo);
    glDeleteBuffers(1, &vbo);
    glDeleteVertexArrays(1, &vao);
  }

  Mesh(std::vector<Vertex> vertices_, std::vector<uint32_t> indices_) : vertices(std::move(vertices_)), indices(std::move(indices_)) {
    glGenBuffers(1, &vbo);
    glGenBuffers(1, &ebo);
    glGenVertexArrays(1, &vao);

    glBindBuffer(GL_ARRAY_BUFFER, vbo);
      glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(vertices[0]), vertices.data(), GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    // should not work, but
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
      glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(indices[0]), indices.data(), GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    glBindVertexArray(vao);
      glEnableVertexAttribArray(0); // positions
      glEnableVertexAttribArray(1); // colors
      glEnableVertexAttribArray(2); // UVs
      glEnableVertexAttribArray(3); // normals

      glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), reinterpret_cast<void*>(0));
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), reinterpret_cast<void*>(3 * sizeof(float)));
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), reinterpret_cast<void*>(6 * sizeof(float)));
        glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), reinterpret_cast<void*>(8 * sizeof(float)));
      glBindBuffer(GL_ARRAY_BUFFER, 0);

      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo); // indices
    glBindVertexArray(0);
  }

  template <typename TFunc>
  static Mesh createMeshByVertexGenerator(size_t size, TFunc f) {
    std::vector<Vertex> vertices;
    std::vector<uint32_t> indices;
    for (size_t i = 0; i < size; i++) {
      vertices.push_back(f(i));
      indices.push_back(i);
    }
    return Mesh(std::move(vertices), std::move(indices));
  }

  static Mesh fromPosUV(std::vector<glm::vec3> positions, std::vector<glm::vec2> uvs) {
    assert(positions.size() == uvs.size());
    return createMeshByVertexGenerator(positions.size(), [&](size_t i) {
      Vertex v = {};
      v.pos = positions[i], v.tex_coord = uvs[i];
      return v;
    });
  }

  static Mesh fromPos(std::vector<glm::vec3> positions) {
    std::vector<Vertex> vertices;
    std::vector<uint32_t> indices;
    for (size_t i = 0; i < positions.size(); i++) {
      vertices.push_back(Vertex{positions[i]});
      indices.push_back(i);
    }
    return Mesh(std::move(vertices), std::move(indices));
  }

  static Mesh fromPosNorm(
      const std::vector<glm::vec3>& positions,
      const std::vector<glm::vec3>& norms) {
    assert(positions.size() == norms.size());
    return createMeshByVertexGenerator(positions.size(), [&](size_t i) {
      Vertex v = {};
      v.pos = positions[i], v.normal = norms[i];
      return v;
    });
  }

  void draw() {
    glBindVertexArray(vao);
    glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
  }
};


Mesh loadSimpleObj(std::string path) {
  std::ifstream fin(path);
  assert(fin);
  
  std::vector<Mesh::Vertex> vertices;
  std::vector<uint32_t> indices;

  std::vector<glm::vec3> positions;
  std::vector<glm::vec2> uvs;
  std::vector<glm::vec3> normals;

  while (fin) {
    std::string kind;
    if (!(fin >> kind))
        break;
    if (kind == "v") {
      glm::vec3 pos;
      fin >> pos.x >> pos.y >> pos.z;
      positions.push_back(pos);
    } else if (kind == "vt") {
      glm::vec2 uv;
      fin >> uv.x >> uv.y;
      uvs.push_back(uv);
    } else if (kind == "f") {
      for (int i = 0; i < 3; i++) {
        std::string vert_indices;
        fin >> vert_indices;

        int pos_idx, uv_idx, norm_idx;
        int scanned = sscanf(vert_indices.c_str(), "%d/%d/%d", &pos_idx, &uv_idx, &norm_idx);
        assert(scanned == 3);

        Mesh::Vertex v = {};

        assert(pos_idx - 1 < positions.size());
        v.pos = positions[pos_idx - 1];

        assert(uv_idx - 1 < uvs.size());
        v.tex_coord = uvs[uv_idx - 1];

        assert(norm_idx - 1 < normals.size());
        v.normal = normals[norm_idx - 1];

        vertices.push_back(v);
        indices.push_back(indices.size());
      }
    } else if (kind == "vn") {
      glm::vec3 norm;
      fin >> norm.x >> norm.y >> norm.z;
      normals.push_back(norm);
    } else {
        assert(kind == "o" || kind == "mtllib" || kind == "#" || kind == "usemtl" || kind == "s");
        std::string line;
        std::getline(fin, line);
        // pass
    }
    assert(fin);
  }

  return Mesh(std::move(vertices), std::move(indices));
}


std::vector<glm::vec3> genCubeVerts() {
  std::vector<glm::vec3> pts;

  auto addSide = [&](glm::quat rot) {
    pts.push_back(rot * glm::vec3{-1, -1, 1});
    pts.push_back(rot * glm::vec3{ 1, -1, 1});
    pts.push_back(rot * glm::vec3{-1,  1, 1});

    pts.push_back(rot * glm::vec3{ 1,  1, 1});
    pts.push_back(rot * glm::vec3{-1,  1, 1});
    pts.push_back(rot * glm::vec3{ 1, -1, 1});
  };

  for (float rots : {0., 0.25, 0.5, 0.75})
    addSide(glm::angleAxis(glm::two_pi<float>() * rots, glm::vec3(0, 1, 0)));
  for (float rots : {-0.25, 0.25})
    addSide(glm::angleAxis(glm::two_pi<float>() * rots, glm::vec3(1, 0, 0)));

  return pts;
}

Mesh genCube() {
  return Mesh::fromPos(genCubeVerts());
}

Mesh genSquareSurface() {
  std::vector<glm::vec3> pts;

  pts.emplace_back(-1, 0, -1);
  pts.emplace_back(1, 0, -1);
  pts.emplace_back(1, 0, 1);

  pts.emplace_back(-1, 0, -1);
  pts.emplace_back(1, 0, 1);
  pts.emplace_back(-1, 0, 1);

  std::vector<glm::vec3> norms(6, {0, 1, 0});

  return Mesh::fromPosNorm(pts, norms);
}
