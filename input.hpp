#pragma once

#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <optional>

using namespace glm;

/**
 * GLFW wrapper but robust to window swithing/alt-tabs/etc
 */
class MouseInput {
public:
  static void initGlobal(GLFWwindow *window, MouseInput &inp) {
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

struct InputContext {
    GLFWwindow *window;
    MouseInput mouse_input;
};