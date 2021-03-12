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
     * 
     */

    struct Vertex {
        glm::vec3 pos;
        glm::vec3 color;
    };
    static_assert(sizeof(Vertex) == sizeof(float) * 6);

    std::vector<Vertex> vertices;
    std::vector<uint32_t> indices;
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

    Mesh(std::vector<Vertex> vertices_, std::vector<uint32_t> indices_): vertices(std::move(vertices_)), indices(std::move(indices_)) {
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
            glBindBuffer(GL_ARRAY_BUFFER, vbo);
                glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), reinterpret_cast<void*>(0));
                glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), reinterpret_cast<void*>(3 * sizeof(float)));
            glBindBuffer(GL_ARRAY_BUFFER, 0);

            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo); // indices
        glBindVertexArray(0);
    }

    static Mesh fromPosColor(std::vector<glm::vec3> positions, std::vector<glm::vec3> colors = {}) {
        assert(positions.size() == colors.size());
        std::vector<Vertex> vertices;
        std::vector<uint32_t> indices;
        for (size_t i = 0; i < positions.size(); i++) {
            vertices.push_back(Vertex{positions[i], colors[i]});
            indices.push_back(i);
        }
        return Mesh(std::move(vertices), std::move(indices));
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

Mesh genCube() {
    return Mesh::fromPosColor(genCubeVerts(), genCubeColors());
}

Mesh genXZPlane() {
    return Mesh::fromPosColor(
        {
            {-1, 0, -1},
            { 1, 0, -1},
            {-1, 0,  1},
            { 1, 0,  1},
            {-1, 0,  1},
            { 1, 0, -1},
        },
        std::vector<glm::vec3>(6, glm::vec3{0.0, 0.0, 0.0})
    );
}

Mesh loadSimpleObj(std::string path) {
    std::ifstream fin(path);
    assert(fin);
    
    std::vector<Mesh::Vertex> vertices;
    std::vector<uint32_t> indices;

    int i = 0;
    while (i++, true) {
        std::string kind;
        if (!(fin >> kind))
            break;
        if (kind == "v") {
            glm::vec3 pos;
            fin >> pos.x >> pos.y >> pos.z;
            vertices.push_back(Mesh::Vertex{
                pos,
                glm::vec3{0.5, 0.5, 0.5}
                // glm::vec3{
                //     (float)(bool)(i % 3 == 0),
                //     (float)(bool)(i % 3 == 1),
                //     (float)(bool)(i % 3 == 2),
                // }
            });
        } else {
            assert(kind == "f");
            for (int i = 0; i < 3; i++) {
                indices.push_back(0);
                fin >> indices.back();
                indices.back()--;
            }
        }
        assert(fin);
    }

    return Mesh(std::move(vertices), std::move(indices));
}