#pragma once

#include <GL/glew.h>
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <unistd.h>

inline std::string readfile(const std::string &path) {
  std::ifstream fin(path);
  fin.exceptions(std::ifstream::failbit | std::ifstream::badbit);
  std::ostringstream contents;
  contents << fin.rdbuf();
  return contents.str();
}

inline uint createShader(std::string path, uint shader_type) {
  uint shader = glCreateShader(shader_type);

  std::string src = readfile(path);
  const char *src_cstr = src.c_str();
  glShaderSource(shader, 1, &src_cstr, nullptr);
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

inline uint createShaderProgram(std::string vertexPath, std::string fragmentPath, std::string geometryPath="") {
  uint vertexShader = createShader(vertexPath, GL_VERTEX_SHADER);
  uint fragmentShader = createShader(fragmentPath, GL_FRAGMENT_SHADER);
  uint geometryShader = (!geometryPath.empty() ? createShader(geometryPath, GL_GEOMETRY_SHADER) : 0);

  uint shaderProgram = glCreateProgram();
  glAttachShader(shaderProgram, vertexShader);
  glAttachShader(shaderProgram, fragmentShader);
  if (!geometryPath.empty())
    glAttachShader(shaderProgram, geometryShader);
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
  if (!geometryPath.empty())
    glDeleteShader(geometryShader);

  return shaderProgram;
}
