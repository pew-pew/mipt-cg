#pragma once

#include <glm/glm.hpp>
#include <vector>
#include <GL/glew.h>
#include <fstream>
#include <cassert>

struct Mesh {
  /**
   * VAO attributes:
   * 0 - vec3 position
   * 1 - vec3 color
   * 2 - vec2 texture coords
   */

  struct Vertex {
    glm::vec3 pos;
    glm::vec3 color;
    glm::vec2 tex_coord;
  };
  static_assert(sizeof(Vertex) == sizeof(float) * 8);

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

      glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), reinterpret_cast<void*>(0));
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), reinterpret_cast<void*>(3 * sizeof(float)));
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), reinterpret_cast<void*>(6 * sizeof(float)));
      glBindBuffer(GL_ARRAY_BUFFER, 0);

      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo); // indices
    glBindVertexArray(0);
  }

  template <typename TFunc>
  static Mesh shitfuck(size_t size, TFunc f) {
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
    return shitfuck(positions.size(), [&](size_t i) {
      Vertex v = {};
      v.pos = positions[i], v.tex_coord = uvs[i];
      return v;
    });
  }

  void draw() {
    glBindVertexArray(vao);
    glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
  }
};

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

std::vector<glm::vec3> genCubeColors() {
  std::vector<glm::vec3> colors;

  auto addSide = [&](glm::vec3 color) {
    for (int i = 0; i < 6; i++) {
      colors.emplace_back(color);
    }
  };

  glm::vec3 c1{1, 0, 0}, c2{0, 1, 0}, c3{0, 0, 1};
  addSide(c1);
  addSide(c2);
  addSide(c1);
  addSide(c2);
  addSide(c3);
  addSide(c3);
  return colors;
}

std::vector<glm::vec2> genCubeUVs() {
  std::vector<glm::vec2> tex;

  auto addSide = [&]() {
    tex.push_back(glm::vec2{0, 0});
    tex.push_back(glm::vec2{1, 0});
    tex.push_back(glm::vec2{0, 1});

    tex.push_back(glm::vec2{1, 1});
    tex.push_back(glm::vec2{0, 1});
    tex.push_back(glm::vec2{1, 0});
  };

  for (int i = 0; i < 6; i++)
    addSide();
  
  return tex;
}


Mesh genCube() {
    return Mesh::fromPosUV(genCubeVerts(), genCubeUVs());
}

// Mesh genXZPlane() {
//     return Mesh::fromPosColor(
//         {
//             {-1, 0, -1},
//             { 1, 0, -1},
//             {-1, 0,  1},
//             { 1, 0,  1},
//             {-1, 0,  1},
//             { 1, 0, -1},
//         },
//         std::vector<glm::vec3>(6, glm::vec3{0.0, 0.0, 0.0})
//     );
// }

Mesh loadSimpleObj(std::string path) {
  std::ifstream fin(path);
  assert(fin);
  
  std::vector<Mesh::Vertex> vertices;
  std::vector<uint32_t> indices;

  std::vector<glm::vec3> positions;
  std::vector<glm::vec2> uvs;

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
        
        Mesh::Vertex v = {};
        size_t s1 = vert_indices.find('/');
        int pos_idx = std::atoi(vert_indices.substr(0, s1).c_str()) - 1;
        assert(pos_idx < positions.size());
        v.pos = positions[pos_idx];

        if (s1 != std::string::npos) {
          size_t s2 = vert_indices.find('/', s1 + 1);
          if (s2 == std::string::npos)
            s2 = vert_indices.size();
          int uv_idx = std::atoi(vert_indices.substr(s1 + 1, s2 - s1 - 1).c_str()) - 1;
          assert(uv_idx < uvs.size());
          v.tex_coord = uvs[uv_idx];
        }

        vertices.push_back(v);
        indices.push_back(indices.size());
      }
    } else {
      assert(kind == "o" || kind == "mtllib" || kind == "#" || kind == "vn" || kind == "usemtl" || kind == "s");
      std::string line;
      std::getline(fin, line);
      //pass
    }
    assert(fin);
  }

  return Mesh(std::move(vertices), std::move(indices));
}