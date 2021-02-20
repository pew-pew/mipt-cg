cmake_minimum_required(VERSION 3.15)
project(mipt-cg)

find_package(glfw3 REQUIRED)
find_package(GLEW REQUIRED)

add_executable(main main.cpp)
target_link_libraries(main glfw GLEW GL)

add_executable(empty empty.cpp)