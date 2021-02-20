cmake_minimum_required(VERSION 3.15)
project(mipt-cg)

include(CMakePrintHelpers)
cmake_print_variables(CMAKE_CXX_COMPILER)
cmake_print_variables(EMSCRIPTEN)
cmake_print_variables(CMAKE_EXECUTABLE_SUFFIX)

if (NOT EMSCRIPTEN)
    add_subdirectory(external/glfw)
    add_subdirectory(external/glew-cmake)
#    find_package(glfw3 REQUIRED)
#    find_package(GLEW REQUIRED)
endif()

add_subdirectory(external/glm)

add_executable(main main.cpp)
target_link_libraries(main glm)

if (NOT EMSCRIPTEN)
    target_link_libraries(main glfw libglew_static GL)
else()
    target_link_options(main PUBLIC -s USE_GLFW=3)
    set_target_properties(main PROPERTIES SUFFIX ".html")
endif()