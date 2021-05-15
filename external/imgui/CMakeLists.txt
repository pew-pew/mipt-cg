# Plain imgui

add_library(
  imgui

  ./imgui/imgui_demo.cpp
  ./imgui/imgui_draw.cpp
  ./imgui/imgui_tables.cpp
  ./imgui/imgui_widgets.cpp
  ./imgui/imgui.cpp
)

target_include_directories(
  imgui
  PUBLIC
  ./imgui
)

# GLFW + gl3 implementation

add_library(
  imgui_glfw_gl3
  ./imgui/backends/imgui_impl_opengl3.cpp
  ./imgui/backends/imgui_impl_glfw.cpp
)

target_include_directories(
  imgui_glfw_gl3
  PUBLIC
  ./imgui
  ./imgui/backends
)

target_link_libraries(imgui_glfw_gl3 imgui)

# Bundled example

add_executable(imgui_example ./imgui/examples/example_glfw_opengl3/main.cpp)
target_link_libraries(imgui_example imgui_glfw_gl3 glfw libglew_static GL)