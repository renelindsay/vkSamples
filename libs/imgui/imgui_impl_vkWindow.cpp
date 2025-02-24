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
static ImGuiKey KeyMap[256]{};

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

    KeyMap[KEY_NONE]         = ImGuiKey_None;
    KeyMap[KEY_A]            = ImGuiKey_A;
    KeyMap[KEY_B]            = ImGuiKey_B;
    KeyMap[KEY_C]            = ImGuiKey_C;
    KeyMap[KEY_D]            = ImGuiKey_D;
    KeyMap[KEY_E]            = ImGuiKey_E;
    KeyMap[KEY_F]            = ImGuiKey_F;
    KeyMap[KEY_G]            = ImGuiKey_G;
    KeyMap[KEY_H]            = ImGuiKey_H;
    KeyMap[KEY_I]            = ImGuiKey_I;
    KeyMap[KEY_J]            = ImGuiKey_J;
    KeyMap[KEY_K]            = ImGuiKey_K;
    KeyMap[KEY_L]            = ImGuiKey_L;
    KeyMap[KEY_M]            = ImGuiKey_M;
    KeyMap[KEY_N]            = ImGuiKey_N;
    KeyMap[KEY_O]            = ImGuiKey_O;
    KeyMap[KEY_P]            = ImGuiKey_P;
    KeyMap[KEY_Q]            = ImGuiKey_Q;
    KeyMap[KEY_R]            = ImGuiKey_R;
    KeyMap[KEY_S]            = ImGuiKey_S;
    KeyMap[KEY_T]            = ImGuiKey_T;
    KeyMap[KEY_U]            = ImGuiKey_U;
    KeyMap[KEY_V]            = ImGuiKey_V;
    KeyMap[KEY_W]            = ImGuiKey_W;
    KeyMap[KEY_X]            = ImGuiKey_X;
    KeyMap[KEY_Y]            = ImGuiKey_Y;
    KeyMap[KEY_Z]            = ImGuiKey_Z;
    KeyMap[KEY_1]            = ImGuiKey_1;
    KeyMap[KEY_2]            = ImGuiKey_2;
    KeyMap[KEY_3]            = ImGuiKey_3;
    KeyMap[KEY_4]            = ImGuiKey_4;
    KeyMap[KEY_5]            = ImGuiKey_5;
    KeyMap[KEY_6]            = ImGuiKey_6;
    KeyMap[KEY_7]            = ImGuiKey_7;
    KeyMap[KEY_8]            = ImGuiKey_8;
    KeyMap[KEY_9]            = ImGuiKey_9;
    KeyMap[KEY_0]            = ImGuiKey_0;
    KeyMap[KEY_Enter]        = ImGuiKey_Enter;
    KeyMap[KEY_Escape]       = ImGuiKey_Escape;
    KeyMap[KEY_Delete]       = ImGuiKey_Backspace;
    KeyMap[KEY_Tab]          = ImGuiKey_Tab;
    KeyMap[KEY_Space]        = ImGuiKey_Space;
    KeyMap[KEY_Minus]        = ImGuiKey_Minus;
    KeyMap[KEY_Equals]       = ImGuiKey_Equal;
    KeyMap[KEY_LeftBracket]  = ImGuiKey_LeftBracket;
    KeyMap[KEY_RightBracket] = ImGuiKey_RightBracket;
    KeyMap[KEY_Backslash]    = ImGuiKey_Backslash;
    KeyMap[KEY_Semicolon]    = ImGuiKey_Semicolon;
    KeyMap[KEY_Quote]        = ImGuiKey_Apostrophe;
    KeyMap[KEY_Grave]        = ImGuiKey_GraveAccent;
    KeyMap[KEY_Comma]        = ImGuiKey_Comma;
    KeyMap[KEY_Period]       = ImGuiKey_Period;
    KeyMap[KEY_Slash]        = ImGuiKey_Slash;
    KeyMap[KEY_CapsLock]     = ImGuiKey_CapsLock;
    KeyMap[KEY_F1]           = ImGuiKey_F1;
    KeyMap[KEY_F2]           = ImGuiKey_F2;
    KeyMap[KEY_F3]           = ImGuiKey_F3;
    KeyMap[KEY_F4]           = ImGuiKey_F4;
    KeyMap[KEY_F5]           = ImGuiKey_F5;
    KeyMap[KEY_F6]           = ImGuiKey_F6;
    KeyMap[KEY_F7]           = ImGuiKey_F7;
    KeyMap[KEY_F8]           = ImGuiKey_F8;
    KeyMap[KEY_F9]           = ImGuiKey_F9;
    KeyMap[KEY_F10]          = ImGuiKey_F10;
    KeyMap[KEY_F11]          = ImGuiKey_F11;
    KeyMap[KEY_F12]          = ImGuiKey_F12;
    KeyMap[KEY_PrintScreen]  = ImGuiKey_PrintScreen;
    KeyMap[KEY_ScrollLock]   = ImGuiKey_ScrollLock;
    KeyMap[KEY_Pause]        = ImGuiKey_Pause;
    KeyMap[KEY_Insert]       = ImGuiKey_Insert;
    KeyMap[KEY_Home]         = ImGuiKey_Home;
    KeyMap[KEY_PageUp]       = ImGuiKey_PageUp;
    KeyMap[KEY_DeleteForward]= ImGuiKey_Delete;
    KeyMap[KEY_End]          = ImGuiKey_End;
    KeyMap[KEY_PageDown]     = ImGuiKey_PageDown;
    KeyMap[KEY_Right]        = ImGuiKey_RightArrow;
    KeyMap[KEY_Left]         = ImGuiKey_LeftArrow;
    KeyMap[KEY_Down]         = ImGuiKey_DownArrow;
    KeyMap[KEY_Up]           = ImGuiKey_UpArrow;
    KeyMap[KP_NumLock]       = ImGuiKey_NumLock;
    KeyMap[KP_Divide]        = ImGuiKey_KeypadDivide;
    KeyMap[KP_Multiply]      = ImGuiKey_KeypadMultiply;
    KeyMap[KP_Subtract]      = ImGuiKey_KeypadSubtract;
    KeyMap[KP_Add]           = ImGuiKey_KeypadAdd;
    KeyMap[KP_Enter]         = ImGuiKey_KeypadEnter;
    KeyMap[KP_1]             = ImGuiKey_Keypad1;
    KeyMap[KP_2]             = ImGuiKey_Keypad2;
    KeyMap[KP_3]             = ImGuiKey_Keypad3;
    KeyMap[KP_4]             = ImGuiKey_Keypad4;
    KeyMap[KP_5]             = ImGuiKey_Keypad5;
    KeyMap[KP_6]             = ImGuiKey_Keypad6;
    KeyMap[KP_7]             = ImGuiKey_Keypad7;
    KeyMap[KP_8]             = ImGuiKey_Keypad8;
    KeyMap[KP_9]             = ImGuiKey_Keypad9;
    KeyMap[KP_0]             = ImGuiKey_Keypad0;
    KeyMap[KP_Point]         = ImGuiKey_KeypadDecimal;
    KeyMap[KEY_F13]          = ImGuiKey_F13;
    KeyMap[KEY_F14]          = ImGuiKey_F14;
    KeyMap[KEY_F15]          = ImGuiKey_F15;
    KeyMap[KEY_F16]          = ImGuiKey_F16;
    KeyMap[KEY_F17]          = ImGuiKey_F17;
    KeyMap[KEY_F18]          = ImGuiKey_F18;
    KeyMap[KEY_F19]          = ImGuiKey_F19;
    KeyMap[KEY_F20]          = ImGuiKey_F20;
    KeyMap[KEY_F21]          = ImGuiKey_F21;
    KeyMap[KEY_F22]          = ImGuiKey_F22;
    KeyMap[KEY_F23]          = ImGuiKey_F23;
    KeyMap[KEY_F24]          = ImGuiKey_F24;
    KeyMap[KEY_Menu]         = ImGuiKey_Menu;
    KeyMap[KEY_LeftControl]  = ImGuiKey_LeftCtrl;
    KeyMap[KEY_LeftShift]    = ImGuiKey_LeftShift;
    KeyMap[KEY_LeftAlt]      = ImGuiKey_LeftAlt;
    KeyMap[KEY_LeftGUI]      = ImGuiKey_LeftSuper;
    KeyMap[KEY_RightControl] = ImGuiKey_RightCtrl;
    KeyMap[KEY_RightShift]   = ImGuiKey_RightShift;
    KeyMap[KEY_RightAlt]     = ImGuiKey_RightAlt;
    KeyMap[KEY_RightGUI]     = ImGuiKey_RightSuper;


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

void ImGui_ImplvkWindow_KeyPressed(vkWindow* window, int keycode, int action) {
    ImGuiIO& io = ImGui::GetIO();
    bool down = (action==(int)eDOWN);
    ImGuiKey scancode = KeyMap[keycode];
    io.AddKeyEvent(scancode, down);

    if((keycode == KEY_LeftControl)||(keycode == KEY_RightControl)) io.AddKeyEvent(ImGuiMod_Ctrl,  down);
    if((keycode == KEY_LeftShift)  ||(keycode == KEY_RightShift)  ) io.AddKeyEvent(ImGuiMod_Shift, down);
    if((keycode == KEY_LeftAlt)    ||(keycode == KEY_RightAlt)    ) io.AddKeyEvent(ImGuiMod_Alt,   down);
    if((keycode == KEY_LeftGUI)    ||(keycode == KEY_RightGUI)    ) io.AddKeyEvent(ImGuiMod_Super, down);
}

void ImGui_ImplvkWindow_TextInput(vkWindow* window, const char* str) {
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

