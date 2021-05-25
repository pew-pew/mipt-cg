#pragma once


#include <chrono>
#include <cmath>
#include <iostream>
#include <thread>
#include <functional>
#include <optional>
#include <vector>
#include <chrono>
#include <algorithm>
#include <cstdlib>
#include <random>

#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp> // translate, rotate, scale, perspective
#include <glm/gtc/type_ptr.hpp> // value_ptr
#include <glm/gtx/string_cast.hpp>
#include <glm/gtx/transform.hpp>
using namespace glm;

#include "input.hpp"

struct QuatTransform {
  glm::vec3 pos;
  glm::quat dir;

  glm::mat4 getMat() {
    return glm::translate(pos) * glm::mat4(dir);
  }
};

struct AngleTransform {
  glm::vec3 pos;
  float horizontal_angle;
  float vertical_angle;

  [[nodiscard]] glm::quat getDir() const {
    return glm::angleAxis(-horizontal_angle, glm::vec3{0, 1, 0})
            * glm::angleAxis(vertical_angle, glm::vec3{1, 0, 0});
  }

  [[nodiscard]] glm::quat getForwardDir() const {
    return glm::angleAxis(-horizontal_angle, glm::vec3{0, 1, 0});
  }
};

struct DyingObject {
  enum class Kind { enemy, projectile };
  QuatTransform transform;
  glm::vec3 explosion_pos;
  glm::vec3 explosion_dir;
  Kind kind;
  double death_start;
  constexpr static double death_duration = 1;
};

class Scene {
 public:
  Scene(InputContext *input_ctx, int64_t random_seed)
        : input_(input_ctx)
        , random_engine_(random_seed)
        , cursor_(input_ctx->mouse_input.getPos()) {
  }

 public:
  AngleTransform player{
      {0, 0, 0}, 0.0f, 0.0f
  };
  std::vector<QuatTransform> enemies;
  std::vector<QuatTransform> projectiles;
  std::vector<DyingObject> dying_objects;
  int killed_count = 0;

  static constexpr glm::vec3 PERSON_HEAD{0, 1.35, 0};

  void update(double elapsed_time) {
    time_ += elapsed_time;
    movePlayer(elapsed_time);
    spawnEnemies(elapsed_time);
    moveProjectiles(elapsed_time);
    checkCollisions();
  }

  void spawnProjectile() {
    projectiles.push_back(QuatTransform{
        player.pos + PERSON_HEAD + player.getDir() * FORWARD * 0.2f,
        player.getDir()
    });
  }

 private:
  void spawnEnemies(double elapsed_time) {
    elapsed_since_last_enemy_spawn_ += elapsed_time;
    if (elapsed_since_last_enemy_spawn_ < SPAWN_DELAY || enemies.size() >= MAX_ENEMIES) {
      return;
    }

    elapsed_since_last_enemy_spawn_ = 0;
    float dist = std::uniform_real_distribution(2.0f, 5.0f)(random_engine_);
    float wing = glm::pi<float>() * 2 * 0.2;
    float ang = std::uniform_real_distribution(-wing, +wing)(random_engine_);

    glm::vec3 enemy_pos = (
        player.pos +
        player.getForwardDir() * glm::angleAxis(ang, glm::vec3{0, 1, 0}) * glm::vec3{0, 0, -1} * dist
    );

    float enemy_rot = std::uniform_real_distribution(0.0f, glm::pi<float>() * 2)(random_engine_) - glm::pi<float>();
    glm::quat enemy_dir = glm::angleAxis(enemy_rot, glm::vec3{0, 1, 0});

    enemies.push_back(QuatTransform{enemy_pos, enemy_dir});
  }

  void movePlayer(double elapsed_time) {
    glm::vec3 delta;
    if (glfwGetKey(input_->window, GLFW_KEY_D) == GLFW_PRESS)
      delta += RIGHT;
    if (glfwGetKey(input_->window, GLFW_KEY_A) == GLFW_PRESS)
      delta += -RIGHT;

    if (glfwGetKey(input_->window, GLFW_KEY_W) == GLFW_PRESS)
      delta += FORWARD;
    if (glfwGetKey(input_->window, GLFW_KEY_S) == GLFW_PRESS)
      delta += -FORWARD;

    if (glfwGetKey(input_->window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS)
      delta *= 0.1;

    glm::vec2 cursor_delta = input_->mouse_input.getPos() - cursor_;
    cursor_ = input_->mouse_input.getPos();
    double horizontal_angle_shift = glm::pi<double>() * 2 * cursor_delta.x / X_FULL_CURSOR_ROTATION;
    double vertical_angle_shift = -glm::pi<double>() * 2 * cursor_delta.y / Y_FULL_CURSOR_ROTATION;

    player.horizontal_angle += horizontal_angle_shift;
    player.vertical_angle = glm::clamp(player.vertical_angle + vertical_angle_shift, MIN_PLAYER_VERTICAL_ANGLE, MAX_PLAYER_VERTICAL_ANGLE);
    
    player.pos += player.getForwardDir() * delta * (float)elapsed_time * PLAYER_MOVE_SPEED;
  }

  void moveProjectiles(double elapsed_time) {
    for (auto& projectile : projectiles) {
      projectile.pos += projectile.dir * FORWARD * (float)elapsed_time * PROJECTILE_MOVE_SPEED;
    }
  }

  void checkCollisions() {
    for (size_t ip = 0; ip < projectiles.size(); ip++) {
      for (size_t ie = 0; ie < enemies.size(); ie++) {
        if (checkCollision(projectiles[ip].pos, enemies[ie].pos)) {
          glm::vec3 expl = projectiles[ip].pos;
          glm::vec3 expl_dir = (projectiles[ip].dir * FORWARD) * PROJECTILE_MOVE_SPEED;
          dying_objects.push_back(DyingObject{enemies[ie], expl, expl_dir, DyingObject::Kind::enemy, time_});
          dying_objects.push_back(DyingObject{projectiles[ip], expl, expl_dir, DyingObject::Kind::projectile, time_});

          enemies.erase(enemies.begin() + ie);
          projectiles.erase(projectiles.begin() + ip);

          killed_count++;
          ip--;
          break;
        }
      }
    }

    for (size_t ip = 0; ip < projectiles.size(); ip++) {
      if (glm::length(projectiles[ip].pos - player.pos) > 100) {
        projectiles.erase(projectiles.begin() + ip);
        ip--;
      }
    }
  }

  static bool checkCollision(const glm::vec3& proj_pos, const glm::vec3& enemy_pos) {
    float top_dist = glm::distance(proj_pos, enemy_pos + PERSON_HEAD);
    float bot_dist = glm::distance(proj_pos, enemy_pos);
    return top_dist + bot_dist < 2;
  }

 private:
  static constexpr double SPAWN_DELAY = 1.0;
  static constexpr int MAX_ENEMIES = 10;

  static constexpr glm::vec3
      UP{0, 1, 0},
      RIGHT{1, 0, 0},
      FORWARD{0, 0, -1};

  static constexpr float PLAYER_MOVE_SPEED = 3;
  static constexpr double X_FULL_CURSOR_ROTATION = 1000;
  static constexpr double Y_FULL_CURSOR_ROTATION = 1000;
  static constexpr double MIN_PLAYER_VERTICAL_ANGLE = -glm::pi<double>() / 2;
  static constexpr double MAX_PLAYER_VERTICAL_ANGLE = glm::pi<double>() / 2;

  static constexpr float PROJECTILE_MOVE_SPEED = 5;

  InputContext *input_;
  std::default_random_engine random_engine_;
  glm::vec2 cursor_;
  double elapsed_since_last_enemy_spawn_{SPAWN_DELAY};
  double time_ = 0;
};
