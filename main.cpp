// Include standard headers
#include <stdio.h>
#include <stdlib.h>
#include <fstream>
#include <sstream>
#include <cassert>
#include <random>

// Include GLEW
#include <GL/glew.h>

// Include GLFW
#include <GLFW/glfw3.h>
GLFWwindow* window;

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

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#include <emscripten/html5.h>
#endif

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp> // translate, rotate, scale, perspective
#include <glm/gtc/type_ptr.hpp> // value_ptr
#include <glm/gtx/string_cast.hpp>
#include <glm/gtx/transform.hpp>
using namespace glm;

#include "utils.hpp"
#include "shader.hpp"
#include "mesh.hpp"

void dumpGLErrors(std::string location="") {
  GLenum err;
  while((err = glGetError()) != GL_NO_ERROR)
  {
    std::cerr << location << " gl error " << err << std::endl;
  }
}
#define GLCHECK dumpGLErrors(__FILE__ ":"  + std::to_string(__LINE__) + ":")

void initGlewGLFW() {
  // Initialise GLFW
  if( !glfwInit() )
  {
    fprintf( stderr, "Failed to initialize GLFW\n" );
    getchar();
    exit(1);
  }

  glfwWindowHint(GLFW_SAMPLES, 4);

#ifndef __EMSCRIPTEN__
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  #ifdef __APPLE__
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // To make MacOS happy; should not be needed
  #endif
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#else
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // To make MacOS happy; should not be needed
  glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_ES_API);
#endif

  window = glfwCreateWindow( 1024, 768, "Sample window", nullptr, nullptr);
  if (window == nullptr) {
    const char* description = nullptr;
    #ifndef __EMSCRIPTEN__
    glfwGetError(&description);
    #else
    description = "(no description available)";
    #endif
    std::cerr << description << "\n"
              << "Failed to open GLFW window. If you have an Intel GPU, they are not 3.3 compatible. Try the 2.1 version of the tutorials." << std::endl;
    getchar();
    glfwTerminate();
    exit(1);
  }

  glfwMakeContextCurrent(window);

  glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
  // if (glfwRawMouseMotionSupported()) {
  //   glfwSetInputMode(window, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);
  // }

  if (glewInit() != GLEW_OK) {
    fprintf(stderr, "Failed to initialize GLEW\n");
    getchar();
    glfwTerminate();
    exit(1);
  }
}

struct QuatTransform {
  glm::vec3 pos;
  glm::quat dir;
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

/**
 * GLFW wrapper but robust to window swithing/alt-tabs/etc 
 */
class MouseInput {
public:
  static void initGlobal(MouseInput &inp) {
    static MouseInput& globalGuy = inp;
    glfwSetCursorPosCallback(window, [](GLFWwindow *window, double x, double y) {
      globalGuy.onCursorPos(x, y);
    });
    glfwSetCursorEnterCallback(window, [](GLFWwindow *window, int enter) {
      globalGuy.onEnterLeave(enter);
    });
  }

  glm::vec2 getPos() {
    return cam;
  }

private:
  glm::vec2 cam{0, 0};
  std::optional<glm::vec2> cursor = std::nullopt;
  bool is_in_window = false;

  void onEnterLeave(bool entered) {
    is_in_window = entered;
    cursor = std::nullopt;
    // glfwGetCursorPos(window, &x, &y); // has old value
  }

  void onCursorPos(double nx, double ny) {
    if (!is_in_window)
      return;
    glm::vec2 cursor_new{nx, ny};
    if (cursor.has_value())
      cam += cursor_new - cursor.value();
    cursor = cursor_new;
  }
};

MouseInput mouse_input;

class Scene {
 public:
  MouseInput *inp_;

  Scene(int64_t random_seed)
        : random_engine_(random_seed)
        , x_cursor_(mouse_input.getPos().x)
        , y_cursor_(mouse_input.getPos().y)
  {
  }

 public:
  AngleTransform player{
      {0, 2, 0}, 0.0f, 0.0f
  };
  std::vector<QuatTransform> enemies;

  void update(double elapsed_time) {
    movePlayer(elapsed_time);
    spawnEnemies(elapsed_time);
  }

 private:
  void spawnEnemies(double elapsed_time) {
    elapsed_since_last_spawn_ += elapsed_time;
    if (elapsed_since_last_spawn_ < SPAWN_DELAY || enemies.size() >= MAX_ENEMIES) {
      return;
    }

    elapsed_since_last_spawn_ = 0;
    float dist = std::uniform_real_distribution(2.0f, 5.0f)(random_engine_);
    float wing = glm::pi<float>() * 2 * 0.2;
    float ang = std::uniform_real_distribution(-wing, +wing)(random_engine_);

    glm::vec3 enemy_pos = (
        player.pos +
        player.getForwardDir() * glm::angleAxis(ang, glm::vec3{0, 1, 0}) * glm::vec3{0, 0, -1} * dist
    );

    float enemy_rot = std::uniform_real_distribution(0.0f, glm::pi<float>() * 2)(random_engine_);
    glm::quat enemy_dir = glm::angleAxis(enemy_rot, glm::vec3{0, 1, 0});

    enemies.push_back(QuatTransform{enemy_pos, enemy_dir});
  }

  void movePlayer(double elapsed_time) {
    constexpr glm::vec3
      up{0, 1, 0},
      right{1, 0, 0},
      forward{0, 0, -1};

    glm::vec3 delta;
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
      delta += right;
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
      delta += -right;

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
      delta += forward;
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
      delta += -forward;

    double dx = mouse_input.getPos().x - x_cursor_;
    double dy = mouse_input.getPos().y - y_cursor_;
    x_cursor_ = mouse_input.getPos().x;
    y_cursor_ = mouse_input.getPos().y;
    double horizontal_angle_shift = glm::pi<double>() * 2 * dx / X_FULL_CURSOR_ROTATION;
    double vertical_angle_shift = -glm::pi<double>() * 2 * dy / Y_FULL_CURSOR_ROTATION;

    player.horizontal_angle += horizontal_angle_shift;
    player.vertical_angle = glm::clamp(player.vertical_angle + vertical_angle_shift, MIN_PLAYER_VERTICAL_ANGLE, MAX_PLAYER_VERTICAL_ANGLE);
    
    player.pos += player.getForwardDir() * delta * (float)elapsed_time * PLAYER_MOVE_SPEED;
  }

 private:
  static constexpr double SPAWN_DELAY = 1.0;
  static constexpr float PLAYER_MOVE_SPEED = 3;
  static constexpr double X_FULL_CURSOR_ROTATION = 1000;
  static constexpr double Y_FULL_CURSOR_ROTATION = 1000;
  static constexpr double MIN_PLAYER_VERTICAL_ANGLE = -glm::pi<double>() / 2;
  static constexpr double MAX_PLAYER_VERTICAL_ANGLE = glm::pi<double>() / 2;
  static constexpr int MAX_ENEMIES = 10;

  std::default_random_engine random_engine_;
  double x_cursor_;
  double y_cursor_;
  double elapsed_since_last_spawn_{SPAWN_DELAY};
};

int main()
{
  initGlewGLFW();

  static int width, height;
  glfwGetWindowSize(window, &width, &height);
  glfwSetFramebufferSizeCallback(window, [](GLFWwindow *window, int w, int h) {
    width = w;
    height = h;
    glViewport(0, 0, w, h);
  });

  uint shader_program = createShaderProgram("./shaders/vertex.glsl", "./shaders/fragment.glsl");
  int mvp_matrix_id = glGetUniformLocation(shader_program, "MVP");
  int texture_id = glGetUniformLocation(shader_program, "tex");

  Mesh roma = loadSimpleObj("./data/roma_smol.obj");
  uint texture_handle = loadTexture("./data/roma_smol.jpg");

  // Fixed FPS
  int targetFPS = 60;
  double timePerFrame = 1.0 / targetFPS;
  double current_time = 0;
  double last_time = 0;
  double elapsed_time = 0;

  glm::vec3 player_camera_shift{0, 1.5, 0};

  MouseInput::initGlobal(mouse_input);
  Scene scene(42);

  auto drawScene = [&]() {
    glm::vec3 player_camera_pos = scene.player.pos + player_camera_shift;
    glm::mat4 viewproj = (
      glm::perspective<float>(glm::radians(60.), (float)width / height, 0.01, 100)
      * glm::lookAt(player_camera_pos,
                    player_camera_pos + scene.player.getDir() * glm::vec3{0, 0, -1},
                    scene.player.getDir() * glm::vec3{0, 1, 0})
    );
    glm::mat4 trans;

    glUseProgram(shader_program);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture_handle);
    glUniform1i(texture_id, 0);

    for (auto& enemy : scene.enemies) {
      glm::mat4 MVP = viewproj
          * glm::translate(enemy.pos)
          * glm::mat4(enemy.dir)
          * glm::scale(glm::vec3{1, 1, 1});

      glUniformMatrix4fv(mvp_matrix_id, 1, GL_FALSE, glm::value_ptr(MVP));
      roma.draw();
    }
  };

  glEnable(GL_DEPTH_TEST);
  glClearColor(0.0f, 0.0f, 0.4f, 0.0f);

  static std::function<void()> loop = [&]() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    last_time = current_time;

    scene.update(elapsed_time);
    drawScene();

    glfwSwapBuffers(window);
    glfwPollEvents();

    current_time = glfwGetTime();
    elapsed_time = current_time - last_time;
    double free_time = std::max(timePerFrame - elapsed_time, 0.0);

    // Sleep until next frame to keep FPS rate at the certain level
    if (free_time > 0) {
      std::this_thread::sleep_for(std::chrono::duration<double>(free_time));
    }
  };

  current_time = glfwGetTime();
  last_time = current_time;

#ifdef __EMSCRIPTEN__
  emscripten_set_main_loop([]() { loop(); }, -1, true);
#else
  do {
    loop();
  } while (glfwWindowShouldClose(window) == 0);
#endif

  // todo: cleanup opengl things

  glfwTerminate();
  return 0;
}
