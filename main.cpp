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
#include <glm/gtc/matrix_transform.hpp> // translate, rotate, scale, perspective
#include <glm/gtc/type_ptr.hpp> // value_ptr
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

  static bool stop = false;
  glfwSetKeyCallback(window, [](GLFWwindow* w, int key, int scancode, int action, int mods) {
    // stop = true;
    std::cout << "KEY!" << std::endl;
  });

  float vertices[] = {
      -0.5, -0.5, 0,
       0.5, -0.5, 0,
       0,    0.5, 0,

      -0.5,  0.5, 0.5,
       0.5,  0.5, 0.5,
       0,   -0.5, 0.5,
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

  uint indices[] = {
    3, 4, 5,
    0, 1, 2,
  };

  uint ebo = 0;
  glGenBuffers(1, &ebo);

  uint vao_el = 0;
  glGenVertexArrays(1, &vao_el);

  glBindVertexArray(vao_el);
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
      glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), reinterpret_cast<void*>(0));
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
  glBindVertexArray(0);

  uint shaderProgram = createShaderProgram("./vertex.glsl", "./fragment.glsl");
  int transLoc = glGetUniformLocation(shaderProgram, "trans");

  glm::mat4 trans = glm::mat4(1.0f);
  glUseProgram(shaderProgram);
    glUniformMatrix4fv(transLoc, 1, false, glm::value_ptr(trans));
  glUseProgram(0);


  glm::vec3 pos = {0, 0, 0}; //std::sin(t), std::cos(t)};

  double lastT = 0;
  static std::function<void()> loop = [&]() {
    glClear(GL_COLOR_BUFFER_BIT);

    glUseProgram(shaderProgram);

    double t = glfwGetTime();
    float dt = t - lastT;
    lastT = t;

    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
      pos += glm::vec3(1, 0, 0) * dt;
    }
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
      pos += glm::vec3(-1, 0, 0) * dt;
    }
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
      pos += glm::vec3(0, 0, -1) * dt;
    }
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
      pos += glm::vec3(0, 0, 1) * dt;
    }
    if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS) {
      pos += glm::vec3(0, 1, 0) * dt;
    }
    if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS) {
      pos += glm::vec3(0, -1, 0) * dt;
    }

    if (!stop) {
      glm::mat4 trans = glm::mat4(1.0f);
      //trans = glm::translate(trans, pos);
      trans = glm::lookAt(pos, pos + glm::vec3{0, 0, -1}, glm::vec3{0, 1, 0});
      trans = glm::perspective<float>(glm::radians(60.), 1, 0.1, 100) * trans;
      glUniformMatrix4fv(transLoc, 1, false, glm::value_ptr(trans));
    }

    glBindVertexArray(vao_el);
    // glDrawArrays(GL_TRIANGLES, 0, 3);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

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
  } while (glfwWindowShouldClose(window) == 0);
#endif

  // todo: cleanup opengl things

  // Close OpenGL window and terminate GLFW
  glfwTerminate();

  return 0;
}
