// Include standard headers
#include <stdio.h>
#include <stdlib.h>
#include <fstream>
#include <sstream>
#include <cassert>

// Include GLEW
#include <GL/glew.h>

// Include GLFW
#include <GLFW/glfw3.h>
GLFWwindow* window;

#include <chrono>
#include <iostream>
#include <thread>
#include <functional>
#include <vector>
#include <chrono>

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
  // glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_COMPAT_PROFILE);
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

  if (glewInit() != GLEW_OK) {
    fprintf(stderr, "Failed to initialize GLEW\n");
    getchar();
    glfwTerminate();
    exit(1);
  }
}

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

  Mesh teapot = loadSimpleObj("./data/teapot.obj");
  Mesh cube = genCube();
  Mesh plane = genXZPlane();
  std::vector<glm::quat> plane_rots = {glm::quat()};
  for (float rot : {0., 0.25, 0.5, 0.75})
    plane_rots.push_back(
      glm::angleAxis(glm::two_pi<float>() * rot, glm::vec3(0, 1, 0))
      * glm::angleAxis(glm::half_pi<float>(), glm::vec3{1, 0, 0})
    );

  uint shaderProgram = createShaderProgram("./shaders/vertex.glsl", "./shaders/fragment.glsl");
  int transLoc = glGetUniformLocation(shaderProgram, "trans");
  int alphaLoc = glGetUniformLocation(shaderProgram, "alpha");

  glm::vec3 pos = {2, 2, 6};
  glm::quat dir;
  double lastT = glfwGetTime();
  auto handleControls = [&]() {
    constexpr glm::vec3
      up{0, 1, 0},
      right{1, 0, 0},
      forward{0, 0, -1};

    double t = glfwGetTime();
    float dt = t - lastT;
    lastT = t;

    float move_speed = 2;
    float rot_speed = 2;

    glm::vec3 delta;
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
      delta += right;
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
      delta += -right;

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
      delta += forward;
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
      delta += -forward;

    if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
      delta += up;
    if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS)
      delta += -up;
    pos += dir * delta * dt * move_speed;

    float rot = dt * rot_speed;
    if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
      dir = glm::rotate(dir, -rot, up);
    if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)
      dir = glm::rotate(dir, rot, up);

    if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
      dir = glm::rotate(dir, rot, right);
    if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
      dir = glm::rotate(dir, -rot, right);

    if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
      dir = glm::rotate(dir, rot, forward);
    if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
      dir = glm::rotate(dir, -rot, forward);
  };

  glEnable(GL_DEPTH_TEST);
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glClearColor(0.0f, 0.0f, 0.4f, 0.0f);
// glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

  static std::function<void()> loop = [&]() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    handleControls();
    glm::mat4 viewproj = (
      glm::perspective<float>(glm::radians(60.), (float)width / height, 0.01, 10)
      * glm::lookAt(pos, pos + glm::mat3(dir) * glm::vec3{0, 0, -1}, glm::mat3(dir) * glm::vec3{0, 1, 0})
    );

    glUseProgram(shaderProgram);

    glm::mat4 trans;
    trans = viewproj;
    glUniformMatrix4fv(transLoc, 1, false, glm::value_ptr(trans));
    glUniform1f(alphaLoc, 1);
    cube.draw();
    //teapot.draw();

    std::vector<glm::mat4> plane_transforms;
    for (glm::quat rot : plane_rots)
      plane_transforms.push_back(
        viewproj * glm::translate(glm::vec3{0, -1, 0}) * glm::mat4(rot)
        * glm::scale(glm::vec3{1.5f, 1.5f, 1.5f}) * glm::translate(glm::vec3{0, -1, 0})
      );

    auto key = [&pos](const glm::mat4 &m) {
      glm::vec4 final = m * glm::vec4{0, 0, 0, 1};
      return -(final.z / final.w);
    };

    std::sort(plane_transforms.begin(), plane_transforms.end(), [&key](const glm::mat4 &a, const glm::mat4 &b) {
      return key(a) < key(b);
    });

    for (glm::mat4 planetrans : plane_transforms) {
      trans = planetrans;
      glUniformMatrix4fv(transLoc, 1, false, glm::value_ptr(trans));
      glUniform1f(alphaLoc, 0.5);
      plane.draw();
    }

    glfwSwapBuffers(window);
    glfwPollEvents();
  };

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
