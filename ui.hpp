#pragma once

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "world.hpp"

struct UI {
  UI(GLFWwindow *window) {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 100"); // glsl version
  }

  void draw(float elapsed_time, float timeSpeed, Scene &scene) {
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();

    ImGui::NewFrame();

    {
      int corner = 0;
      bool open = true;
      bool* p_open = &open;
      ImGuiWindowFlags window_flags = (
          ImGuiWindowFlags_NoDecoration
        | ImGuiWindowFlags_AlwaysAutoResize
        | ImGuiWindowFlags_NoSavedSettings
        | ImGuiWindowFlags_NoFocusOnAppearing
        | ImGuiWindowFlags_NoNav
        | ImGuiWindowFlags_NoMove
      );
      ImGui::SetNextWindowPos({10.0f, 10.0f}, ImGuiCond_Always, {0.0f, 0.0f});
      ImGui::SetNextWindowBgAlpha(0.35f); // Transparent background
      if (ImGui::Begin("overlay", p_open, window_flags))
      {
        ImGui::Text("Controls:\nMove - w/a/s/d\nLook - mouse\nShoot - LMB\nTime control - up/down arrows");
        ImGui::Separator();
        ImGui::Text("FPS: %.1f", (elapsed_time ? 1.0f / elapsed_time : 0));
        ImGui::Text("Fuckers alive: %d", (int)scene.enemies.size());
        ImGui::Text("Fuckers killed: %d", scene.killed_count);
        ImGui::Text("Time speed: %.2f", timeSpeed);
      }
      ImGui::End();
    }

    ImGui::EndFrame();
    ImGui::Render();

    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
  }
};