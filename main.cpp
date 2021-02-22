// Include standard headers
#include <stdio.h>
#include <stdlib.h>
#include <fstream>
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

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#include <emscripten/html5.h>
#endif

// Include GLM
#include <glm/glm.hpp>
using namespace glm;

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
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // To make MacOS happy; should not be needed
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#else
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // To make MacOS happy; should not be needed
  glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_ES_API);
#endif

  // Open a window and create its OpenGL context
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

  // Initialize GLEW
  if (glewInit() != GLEW_OK) {
    fprintf(stderr, "Failed to initialize GLEW\n");
    getchar();
    glfwTerminate();
    exit(1);
  }
}

std::string readfile(const std::string &path) {
  std::ifstream fin(path);
  assert(fin);
  return std::string(std::istreambuf_iterator<char>(fin), {});
}

uint createShader(std::string path, uint shader_type) {
  uint shader = glCreateShader(shader_type);

  std::string src_str = readfile(path);
  const char *src = src_str.c_str();
  glShaderSource(shader, 1, &src, nullptr);
  glCompileShader(shader);

  int success = 0;
  glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
  if (!success) {
    char info[512];
    glGetShaderInfoLog(shader, sizeof(info), nullptr, info);
    std::cerr << "Failed to compile shader from " << path << " :\n" << info << std::endl;
    exit(1);
  }
  return shader;
}

uint createShaderProgram(std::string vertexPath, std::string fragmentPath) {
  uint vertexShader = createShader(vertexPath, GL_VERTEX_SHADER);
  uint fragmentShader = createShader(fragmentPath, GL_FRAGMENT_SHADER);

  uint shaderProgram = glCreateProgram();
  glAttachShader(shaderProgram, vertexShader);
  glAttachShader(shaderProgram, fragmentShader);
  glLinkProgram(shaderProgram);
  {
    int success = 0;
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
      char info[512];
      glGetProgramInfoLog(shaderProgram, 512, NULL, info);
      std::cerr << "Failed to link shader program:\n" << info << std::endl;
      exit(1);
    }
  }
  glDeleteShader(vertexShader);
  glDeleteShader(fragmentShader);

  return shaderProgram;
}

void dumpGLErrors() {
  GLenum err;
  while((err = glGetError()) != GL_NO_ERROR)
  {
    std::cerr << "gl error " << err << std::endl;
  }
}

int main( void )
{
  initGlewGLFW();

  glfwSetFramebufferSizeCallback(window, [](GLFWwindow *window, int w, int h) {
    std::cout << "resize " << w << " x " << h << std::endl;
    glViewport(0, 0, w, h);
  });

  // Dark blue background
  glClearColor(0.0f, 0.0f, 0.4f, 0.0f);

  glfwSetKeyCallback(window, [](GLFWwindow* w, int key, int scancode, int action, int mods) {
    std::cout << "KEY!" << std::endl;
  });

  float vertices[] = {
      -0.5, -0.5, 0,
       0.5, -0.5, 0,
       0,    0.5, 0,
  };

  uint vbo = 0;
  glGenBuffers(1, &vbo);

  glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
  glBindBuffer(GL_ARRAY_BUFFER, 0);

  uint vao = 0;
  glGenVertexArrays(1, &vao);

  glBindVertexArray(vao);
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
      glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), reinterpret_cast<void*>(0));
    glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindVertexArray(0);

  uint shaderProgram = createShaderProgram("./vertex.glsl", "./fragment.glsl");

  static std::function<void()> loop = [&]() {
    glClear( GL_COLOR_BUFFER_BIT );

    glUseProgram(shaderProgram);
    glBindVertexArray(vao);
    glDrawArrays(GL_TRIANGLES, 0, 3);

    dumpGLErrors();

    glfwSwapBuffers(window);
    glfwPollEvents();
  };

// glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
#ifdef __EMSCRIPTEN__
  emscripten_set_main_loop([]() { loop(); }, -1, true);
#else
  do {
    loop();
  } while( glfwWindowShouldClose(window) == 0 );
#endif

  // todo: cleanup opengl things

  // Close OpenGL window and terminate GLFW
  glfwTerminate();

  return 0;
}
