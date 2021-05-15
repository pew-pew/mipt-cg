#include <stdio.h>
#include <stdlib.h>
#include <fstream>
#include <sstream>
#include <cassert>
#include <random>

#include <GL/glew.h>

#include <GLFW/glfw3.h>

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
#include "world.hpp"
#include "input.hpp"
#include "graphics.hpp"
#include "ui.hpp"

void dumpGLErrors(std::string location="") {
  GLenum err;
  while((err = glGetError()) != GL_NO_ERROR)
  {
    std::cerr << location << " gl error " << err << std::endl;
  }
}
#define GLCHECK dumpGLErrors(__FILE__ ":"  + std::to_string(__LINE__) + ":")

GLFWwindow* initGlewGLFW() {
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
  glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, true);
#else
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // To make MacOS happy; should not be needed
  glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_ES_API);
#endif

  GLFWwindow *window = glfwCreateWindow( 1024, 768, "Sample window", nullptr, nullptr);
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

  if (glewInit() != GLEW_OK) {
    fprintf(stderr, "Failed to initialize GLEW\n");
    getchar();
    glfwTerminate();
    exit(1);
  }
  return window;
}


int main()
{
  GLFWwindow *window = initGlewGLFW();

  InputContext input{window};
  MouseInput::initGlobal(window, input.mouse_input);

  Scene scene(&input, 42);

  static auto mouse_click_callback = [&](int button, int action) {
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
      scene.spawnProjectile();
  };

  glfwSetMouseButtonCallback(window, [](GLFWwindow* window, int button, int action, int mods) {
    mouse_click_callback(button, action);
  });

  int targetFPS = 60;
  double timePerFrame = 1.0 / targetFPS;
  double current_time = glfwGetTime();;
  double last_time = current_time;
  double game_time = 0;
  double frame_time = 0;

  Graphics graphics;
  Graphics::initGlobal(graphics, window);
  graphics.prepare();

  UI ui(window);

  static double timeSpeed = 1;
  glfwSetKeyCallback(window, [](GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (action != GLFW_PRESS)
      return;

    double step = 1.25;
    if (key == GLFW_KEY_UP)
      timeSpeed *= step;
    else if (key == GLFW_KEY_DOWN)
      timeSpeed /= step;
  });

  static std::function<void()> loop = [&]() {
    last_time = current_time;

    game_time += frame_time * timeSpeed;
    scene.update(frame_time * timeSpeed);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    graphics.drawScene(game_time, scene);
    ui.draw(frame_time, timeSpeed, scene);

    glfwSwapBuffers(window);
    glfwPollEvents();

    current_time = glfwGetTime();
    frame_time = current_time - last_time;
    double free_time = std::max(timePerFrame - frame_time, 0.0);

    // Sleep until next frame to keep FPS rate at the certain level
    #ifndef __EMSCRIPTEN__
    if (free_time > 0) {
      std::this_thread::sleep_for(std::chrono::duration<double>(free_time));
    }
    #endif
  };

#ifdef __EMSCRIPTEN__
  emscripten_set_main_loop([]() { loop(); }, -1, true);
#else
  do {
    loop();
  } while (glfwWindowShouldClose(window) == 0);
#endif

  // todo: cleanup opengl things

  ImGui_ImplOpenGL3_Shutdown();
  ImGui_ImplGlfw_Shutdown();
  ImGui::DestroyContext();

  glfwDestroyWindow(window);
  glfwTerminate();
  return 0;
}
