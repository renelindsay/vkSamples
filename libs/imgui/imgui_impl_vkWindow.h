// dear imgui: Platform Binding for vkWindow
// This needs to be used along with the Vulkan Renderer
// (Info: vkWindow is a cross-platform general purpose library for handling windows and keyboard/mouse/touch-screen inputs)

// You can copy and use unmodified imgui_impl_* files in your project. See main.cpp for an example of using this.
// If you are new to dear imgui, read examples/README.txt and read the documentation at the top of imgui.cpp.
// https://github.com/ocornut/imgui

// About GLSL version:
// The 'glsl_version' initialization parameter defaults to "#version 150" if NULL.
// Only override if your GL version doesn't handle this GLSL version. Keep NULL if unsure!

// TODO: Gamepad / IME / MouseCursor

#pragma once

//---------------------------------
#include <vexel.h>
#include "imgui.h"
//---------------------------------

class vkWindow;

IMGUI_IMPL_API bool     ImGui_ImplvkWindow_Init(vkWindow* window);
IMGUI_IMPL_API void     ImGui_ImplvkWindow_Shutdown();
IMGUI_IMPL_API void     ImGui_ImplvkWindow_NewFrame();

extern float ActualFramerate;  // Wall-clock framerate average over last 120 frames (io.framerate gives cpu-time)

void ImGui_ImplvkWindow_UpdateMouse(uint8_t btns, int16_t x, int16_t y);
void ImGui_ImplvkWindow_ScrollWheel(vkWindow* window, float xoffset, float yoffset);
void ImGui_ImplvkWindow_KeyPressed(vkWindow* window, int keycode, int action);
void ImGui_ImplvkWindow_TextInput(vkWindow* window, const char* str);
