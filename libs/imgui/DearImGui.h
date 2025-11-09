#ifndef DEARIMGUI_H
#define DEARIMGUI_H

#include "CDevices.h"
#include "Swapchain.h"
#include "CRenderpass.h"
#include "VkWindow.h"

#include "imgui.h"
#include "imgui_impl_gWindow.h"
#include "imgui_impl_vulkan.h"
#include "imgui_internal.h"

class DearImGui {
    VkDevice device;
public:
    VkInstance instance;
    VkWindow* window;
    VkSampleCountFlagBits samples = VK_SAMPLE_COUNT_1_BIT;
    bool isActive = false;

    DearImGui(){}
    ~DearImGui(){Destroy();}

    void Init(CQueue& graphics_queue, Swapchain& swapchain, CRenderpass& renderpass) {
        //---Window---
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO();
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;  // Enable Keyboard Controls
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;   // Enable Gamepad Controls
        ImGui::StyleColorsDark();
        io.Fonts->AddFontDefault();
        ImGui_ImplgWindow_Init(window);
        //------------
        //---Vulkan---
        device = graphics_queue.device;
        ImGui_ImplVulkan_InitInfo init_info = {};
        init_info.Instance       = instance;
        init_info.PhysicalDevice = graphics_queue.gpu;
        init_info.Device         = graphics_queue.device;
        init_info.QueueFamily    = graphics_queue.family;
        init_info.Queue          = graphics_queue.queue;
        init_info.DescriptorPool = VK_NULL_HANDLE;
        init_info.DescriptorPoolSize = IMGUI_IMPL_VULKAN_MINIMUM_IMAGE_SAMPLER_POOL_SIZE;
        init_info.MinImageCount  = swapchain.surface_caps.minImageCount;  //2
        init_info.ImageCount     = swapchain.info.minImageCount;          //3
        init_info.PipelineInfoMain.RenderPass  = renderpass;
        init_info.PipelineInfoMain.Subpass     = 1;
        init_info.PipelineInfoMain.MSAASamples = samples;
        ImGui_ImplVulkan_Init(&init_info);
        //-----------
    }

    void Destroy() {
        // ImGui Cleanup
        VKERRCHECK(vkDeviceWaitIdle(device));
        ImGui_ImplVulkan_Shutdown();
        ImGui_ImplgWindow_Shutdown();
        ImGui::DestroyContext();
    }

    bool OnKeyEvent(eAction action, eKeycode keycode) {
        ImGui_ImplgWindow_KeyPressed(window, keycode, action);
        ImGuiIO& io = ImGui::GetIO();
        return io.WantCaptureKeyboard; // return true if imgui handled the key event
    }

    bool OnTextEvent(const char* str) {
        ImGui_ImplgWindow_TextInput(window, str);
        ImGuiIO& io = ImGui::GetIO();
        return io.WantCaptureKeyboard;
    }

    bool OnMouseEvent(eAction action, int16_t x, int16_t y, uint8_t btn) {
        static uint8_t btns = 0;
        if(action==eDOWN)  btns |= 1<<(btn-1);
        if(action==eUP  )  btns &= !(1<<(btn-1));
        ImGui_ImplgWindow_UpdateMouse(btns, x, y);
        if(btn == 4) ImGui_ImplgWindow_ScrollWheel(window, 0, 1.f);
        if(btn == 5) ImGui_ImplgWindow_ScrollWheel(window, 0,-1.f);
        ImGuiIO& io = ImGui::GetIO();
        return io.WantCaptureMouse;  // return true if imgui handled the mouse event
    }

    bool OnTouchEvent(eAction action, float x, float y, uint8_t id) {
        static uint8_t btns = 0;
        if(id==0) {
            if(action==eDOWN) btns = 1;
            if(action==eUP  ) btns = 0;
            ImGui_ImplgWindow_UpdateMouse(btns, x, y);
        }
        ImGuiIO& io = ImGui::GetIO();
        return io.WantCaptureMouse;  // return true if imgui handled the mouse event
    }

    bool HandleMouse() { // dont use (no mousewheel)
        int16_t mx, my;
        window->getMousePos(mx, my);
        uint8_t btns = 0;
        if(window->getBtnState(1)) btns+=1;
        if(window->getBtnState(2)) btns+=2;
        if(window->getBtnState(3)) btns+=4;
        ImGui_ImplgWindow_UpdateMouse(btns, mx, my);
        ImGuiIO& io = ImGui::GetIO();
        return io.WantCaptureMouse;  // return true if imgui handled the mouse event
    }

    void NewFrame() {
        ImGui_ImplVulkan_NewFrame();
        ImGui_ImplgWindow_NewFrame();
        ImGui::NewFrame();
        isActive = true;
    }

    void Render(VkCommandBuffer cmd) {
        if(!isActive) return;
        ImGui::Render();
        ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), cmd);
        isActive = false;
    }
};

#endif //DIMGUI_H
