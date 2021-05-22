#pragma once

#include <GL/glew.h>

#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp> // translate, rotate, scale, perspective
#include <glm/gtc/type_ptr.hpp> // value_ptr
#include <glm/gtx/string_cast.hpp>
#include <glm/gtx/transform.hpp>
using namespace glm;

#include "world.hpp"
#include "shader.hpp"
#include "utils.hpp"
#include "mesh.hpp"

#ifdef __EMSCRIPTEN__
constexpr bool IS_EMSCRIPTEN = true;
#else
constexpr bool IS_EMSCRIPTEN = false;
#endif

struct Graphics {
  // Fixed FPS
  uint shader_program = createShaderProgram(
    (IS_EMSCRIPTEN ? "./shaders/vertex_es.glsl"   : "./shaders/vertex.glsl"),
    (IS_EMSCRIPTEN ? "./shaders/fragment_es.glsl" : "./shaders/fragment.glsl"),
    (IS_EMSCRIPTEN ? "doesnotexist"               : "./shaders/geometry.glsl")
  );

  Mesh roma_mesh = loadSimpleObj("./data/roma_smol.obj");
  uint roma_texture = loadTexture("./data/roma_smol.jpg");

  Mesh projectile_mesh = loadSimpleObj("./data/projectile.obj");
  uint projectile_texture = loadTexture("./data/projectile.jpg");

  GLFWwindow *window;
  int width, height;

  static void initGlobal(Graphics &graphics, GLFWwindow *window) {
    static Graphics &theguy = graphics;
    theguy.window = window;
    glfwGetWindowSize(window, &theguy.width, &theguy.height);
    glfwSetFramebufferSizeCallback(window, [](GLFWwindow *window, int w, int h) {
      theguy.width = w;
      theguy.height = h;
      glViewport(0, 0, w, h);
    });
  }

  void prepare() {
    glEnable(GL_DEPTH_TEST);
    glClearColor(0.0f, 0.0f, 0.4f, 0.0f);
  }

  void drawScene(double current_time, Scene &scene) {
    // I don't care about performance
    int expl_time_id = glGetUniformLocation(shader_program, "explosionTime");
    int expl_total_time_id = glGetUniformLocation(shader_program, "explosionTotalTime");
    int expl_pos_id = glGetUniformLocation(shader_program, "explosionPos_world");
    int expl_dir_id = glGetUniformLocation(shader_program, "explosionDir_world");
    
    int m_matrix_id = glGetUniformLocation(shader_program, "M");
    int v_matrix_id = glGetUniformLocation(shader_program, "V");
    int p_matrix_id = glGetUniformLocation(shader_program, "P");
    int light_pos_id = glGetUniformLocation(shader_program, "light_pos");
    int ambient_id = glGetUniformLocation(shader_program, "ambientK");
    int texture_id = glGetUniformLocation(shader_program, "tex");

    glm::vec3 player_camera_pos = scene.player.pos + Scene::PERSON_HEAD;
    glm::mat4 view = glm::lookAt(player_camera_pos,
                                     player_camera_pos + scene.player.getDir() * glm::vec3{0, 0, -1},
                                     scene.player.getDir() * glm::vec3{0, 1, 0});
    glm::mat4 projection = glm::perspective<float>(glm::radians(60.),
                                                   (float)width / height,
                                                   0.01, 100);
    glm::vec3 light_pos = player_camera_pos;
    if (!scene.projectiles.empty())
      light_pos = scene.projectiles.back().pos;

    glUseProgram(shader_program);
    glUniformMatrix4fv(v_matrix_id, 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(p_matrix_id, 1, GL_FALSE, glm::value_ptr(projection));
    glUniform3fv(light_pos_id, 1, glm::value_ptr(light_pos));

    {
      glm::vec3 dummy{1, 0, 0};
      glUniform3fv(expl_pos_id, 1, glm::value_ptr(dummy));
      glUniform3fv(expl_dir_id, 1, glm::value_ptr(dummy));
      glUniform1f(expl_time_id, 0.0f);
      glUniform1f(expl_total_time_id, 1.0f);
    }

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, roma_texture);
    glUniform1i(texture_id, 0);

    glUniform1f(ambient_id, 0.1f);
    for (auto& enemy_trans : scene.enemies) {
      glm::mat4 model = enemy_trans.getMat();

      glUniformMatrix4fv(m_matrix_id, 1, GL_FALSE, glm::value_ptr(model));
      roma_mesh.draw();
    }

    glUniform1f(ambient_id, 1.0f);

    glBindTexture(GL_TEXTURE_2D, projectile_texture);
    for (auto& proj_trans : scene.projectiles) {
      glm::mat4 model = (
          proj_trans.getMat()
          * glm::rotate(
              (float)current_time * 10,
              glm::vec3{0.1, 0, 1}
              )
          * glm::scale(glm::vec3{1., 1., 1.} / 5.0f)
      );

      glUniformMatrix4fv(m_matrix_id, 1, GL_FALSE, glm::value_ptr(model));
      projectile_mesh.draw();
    }

    for (auto& obj : scene.dying_objects) {
      if (current_time - obj.death_start > obj.death_duration)
        continue;

      glm::mat4 model;
      if (obj.kind == DyingObject::Kind::projectile) {
        model = (
            obj.transform.getMat()
            * glm::rotate(
                // (float)obj.death_start * 10,
                (float)current_time * 10,
                glm::vec3{0.1, 0, 1}
                )
            * glm::scale(glm::vec3{1., 1., 1.} / 5.0f)
        );
      } else {
        model = obj.transform.getMat();
      }
      glUniform3fv(expl_dir_id, 1, glm::value_ptr(obj.explosion_dir));
      glUniform3fv(expl_pos_id, 1, glm::value_ptr(obj.explosion_pos));
      glUniform1f(expl_time_id, (float)(current_time - obj.death_start));
      glUniform1f(expl_total_time_id, (float)obj.death_duration);

      glUniformMatrix4fv(m_matrix_id, 1, GL_FALSE, glm::value_ptr(model));
      if (obj.kind == DyingObject::Kind::projectile) {
        glUniform1f(ambient_id, 1.0f);
        glBindTexture(GL_TEXTURE_2D, projectile_texture);
        projectile_mesh.draw();
      } else {
        glUniform1f(ambient_id, 0.1f);
        glBindTexture(GL_TEXTURE_2D, roma_texture);
        roma_mesh.draw();
      }
    }
  }
};
