// dear imgui: Platform Binding for vkWindow
// This needs to be used along with the Vulkan Renderer
// (Info: vkWindow is a cross-platform general purpose library for handling windows, inputs, OpenGL/Vulkan graphics context creation, etc.)

#include "vkWindow.h"
#include "imgui_impl_vkWindow.h"
#include "imgui.h"
#include <chrono>

float min(float a, float b) { return a>b?b:a; }
float max(float a, float b) { return a>b?a:b; }

static vkWindow* g_window = nullptr;
static float g_scale = 1.f;

static const char* ImGui_ImplvkWindow_GetClipboardText(void* user_data) {
    return g_window->GetClipboardText();
}

static void ImGui_ImplvkWindow_SetClipboardText(void* user_data, const char* text) {
    g_window->SetClipboardText(text);
}

bool ImGui_ImplvkWindow_Init(vkWindow* window) {
    g_window = window;
    g_scale = g_window->GetDisplayScale();

    // Setup back-end capabilities flags
    ImGuiIO& io = ImGui::GetIO();
    //io.BackendFlags |= ImGuiBackendFlags_HasMouseCursors;         // We can honor GetMouseCursor() values (optional)
    //io.BackendFlags |= ImGuiBackendFlags_HasSetMousePos;          // We can honor io.WantSetMousePos requests (optional, rarely used)
    io.BackendPlatformName = "imgui_impl_vkWindow";

    // Keyboard mapping. ImGui will use those indices to peek into the io.KeysDown[] array.
    io.KeyMap[ImGuiKey_Tab]         = KEY_Tab;
    io.KeyMap[ImGuiKey_LeftArrow]   = KEY_Left;
    io.KeyMap[ImGuiKey_RightArrow]  = KEY_Right;
    io.KeyMap[ImGuiKey_UpArrow]     = KEY_Up;
    io.KeyMap[ImGuiKey_DownArrow]   = KEY_Down;
    io.KeyMap[ImGuiKey_PageUp]      = KEY_PageUp;
    io.KeyMap[ImGuiKey_PageDown]    = KEY_PageDown;
    io.KeyMap[ImGuiKey_Home]        = KEY_Home;
    io.KeyMap[ImGuiKey_End]         = KEY_End;
    io.KeyMap[ImGuiKey_Insert]      = KEY_Insert;
    io.KeyMap[ImGuiKey_Delete]      = KEY_DeleteForward;
    io.KeyMap[ImGuiKey_Backspace]   = KEY_Delete;
    io.KeyMap[ImGuiKey_Space]       = KEY_Space;
    io.KeyMap[ImGuiKey_Enter]       = KEY_Enter;
    io.KeyMap[ImGuiKey_Escape]      = KEY_Escape;
    io.KeyMap[ImGuiKey_KeypadEnter] = KP_Enter;
    io.KeyMap[ImGuiKey_A]           = KEY_A;
    io.KeyMap[ImGuiKey_C]           = KEY_C;
    io.KeyMap[ImGuiKey_V]           = KEY_V;
    io.KeyMap[ImGuiKey_X]           = KEY_X;
    io.KeyMap[ImGuiKey_Y]           = KEY_Y;
    io.KeyMap[ImGuiKey_Z]           = KEY_Z;

    io.SetClipboardTextFn = ImGui_ImplvkWindow_SetClipboardText;
    io.GetClipboardTextFn = ImGui_ImplvkWindow_GetClipboardText;
    io.ClipboardUserData = g_window;
#if defined(_WIN32)
    //io.ImeWindowHandle = (void*)glfwGetWin32Window(g_Window);
#endif

    return true;
}

void ImGui_ImplvkWindow_Shutdown() {
    g_window = nullptr;
}

void ImGui_ImplvkWindow_UpdateMouse(uint8_t btns, int16_t x, int16_t y) {
    // Update buttons
    ImGuiIO& io = ImGui::GetIO();
    io.MouseDown[0] = btns & 1;  // left button
    io.MouseDown[1] = btns & 4;  // middle button
    io.MouseDown[2] = btns & 2;  // right button

    // Update mouse position
    if (!io.WantSetMousePos) {
        io.MousePos = ImVec2((float)x / g_scale, (float)y / g_scale);
    }
}

void ImGui_ImplvkWindow_ScrollWheel(vkWindow* window, float xoffset, float yoffset) {
    ImGuiIO& io = ImGui::GetIO();
    io.MouseWheelH += xoffset;
    io.MouseWheel += yoffset;
}

void ImGui_ImplvkWindow_KeyPressed(vkWindow* window, int scancode, int action) {
    ImGuiIO& io = ImGui::GetIO();
    if(action == (int)eDOWN) io.KeysDown[scancode] = true;
    if(action == (int)eUP  ) io.KeysDown[scancode] = false;

    // Modifiers are not reliable across systems
    io.KeyCtrl  = io.KeysDown[KEY_LeftControl] || io.KeysDown[KEY_RightControl];
    io.KeyShift = io.KeysDown[KEY_LeftShift]   || io.KeysDown[KEY_RightShift];
    io.KeyAlt   = io.KeysDown[KEY_LeftAlt]     || io.KeysDown[KEY_RightAlt];
    io.KeySuper = io.KeysDown[KEY_LeftGUI]     || io.KeysDown[KEY_RightGUI];
}

void ImGui_ImplvkWindow_TextInput(const char* str) {
    ImGuiIO& io = ImGui::GetIO();
    unsigned int chr = str[0];
    io.AddInputCharacter(chr);
}
/*
// Similar to GetTickCount, but portable.
// It rolls over every ~12.1 days (0x100000/24/60/60)
uint GetMilliCount() {  // wall time
  timeb tb;
  ftime(&tb);
  return (uint)(tb.millitm+tb.time*1000);
}

float cpu_delta_time() {  // returns CPU time in seconds, since last call
    static clock_t last = 0;
           clock_t curr = clock();
    uint32_t delta = curr - last;
    last = curr;
    return delta / (float)CLOCKS_PER_SEC;
};

float wall_delta_time() {  // returns wall clock time in seconds, since last call (milli-second res)
    static uint last = 0;
           uint curr = GetMilliCount();
     uint32_t delta = curr - last;
     last = curr;
     return delta / 1000.f;
};
*/
double wall_delta_time_hires() {  // returns wall clock time in seconds, since last call (nano-second res)
    using namespace std::chrono;
    static high_resolution_clock::time_point last = high_resolution_clock::now();
           high_resolution_clock::time_point curr = high_resolution_clock::now();
    duration<double> time_span = duration_cast<duration<double>>(curr - last);
    last = curr;
    return time_span.count();
}

float ActualFramerate;  // Wall-clock framerate average over last 120 frames

void ImGui_ImplvkWindow_NewFrame() {
    ImGuiIO& io = ImGui::GetIO();
    IM_ASSERT(io.Fonts->IsBuilt() && "Font atlas not built! It is generally built by the renderer back-end. Missing call to renderer _NewFrame() function? e.g. ImGui_ImplOpenGL3_NewFrame().");

    // Setup display size (every frame to accommodate for window resizing)
    int w, h;
    g_scale = g_window->GetDisplayScale();
    g_window->GetWinSize(w,h);
    io.DisplaySize = ImVec2((float)w, (float)h);
    if (w > 0 && h > 0) io.DisplayFramebufferScale = ImVec2(g_scale ,g_scale);

    //float delta =  max(cpu_delta_time(), 0.0001);
    //float delta =  max(wall_delta_time(), 0.0001);
    float delta =  max((float)wall_delta_time_hires(), 0.0001f);

    io.DeltaTime = delta;
    //printf("%f\n", delta);

    //---Get average framerate over last 120 frames---
    const int count = 120;
    static int ofs = 0;
    static float ftimes[count]{};
    ftimes[ofs++] = delta;
    ofs = ofs % count;

    float rate = 0;
    repeat(120) rate += ftimes[i];
    rate = rate / 120.f;
    rate = 1.f / rate;
    //printf("%f\n", frate);
    //io.ActualFramerate = rate;
    ActualFramerate = rate;
    //------------------------------------------------


    //ImGui_ImplGlfw_UpdateMousePosAndButtons();  //TODO
    //ImGui_ImplGlfw_UpdateMouseCursor();         //TODO
    //ImGui_ImplGlfw_UpdateGamepads();            // Update game controllers (if enabled and available)
}

