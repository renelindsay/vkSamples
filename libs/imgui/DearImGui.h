#ifndef DEARIMGUI_H
#define DEARIMGUI_H

#include "CDevices.h"
#include "Swapchain.h"
#include "CRenderpass.h"
#include "vkWindow.h"

#include "imgui.h"
#include "imgui_impl_vkWindow.h"
#include "imgui_impl_vulkan.h"

class DearImGui {
    VkDevice device;
    VkDescriptorPool descriptorPool = 0;

public:
    VkInstance instance;
    vkWindow* window;
    bool isActive = false;

    DearImGui(){}
    ~DearImGui(){Destroy();}

    void Init(CQueue& graphics_queue, Swapchain& swapchain, CRenderpass& renderpass) {
        device = graphics_queue.device;

        //---DescriptorPool---
        VkDescriptorPoolSize pool_sizes[] = {{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1 },};
        VkDescriptorPoolCreateInfo pool_info = {VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO};
        pool_info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
        pool_info.maxSets = 1;
        pool_info.poolSizeCount = (uint32_t)IM_ARRAYSIZE(pool_sizes);
        pool_info.pPoolSizes = pool_sizes;
        VkResult err = vkCreateDescriptorPool(device, &pool_info, NULL, &descriptorPool);
        VKERRCHECK(err);
        //--------------------

        //---ImGui---
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO();
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
        //io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;   // Enable Gamepad Controls
        ImGui::StyleColorsDark();

        io.Fonts->AddFontDefault();

        ImGui_ImplvkWindow_Init(window);

        ImGui_ImplVulkan_InitInfo init_info = {};
        init_info.Instance       = instance;
        init_info.PhysicalDevice = graphics_queue.gpu;
        init_info.Device         = graphics_queue.device;
        init_info.QueueFamily    = graphics_queue.family;
        init_info.Queue          = graphics_queue.queue;
        init_info.DescriptorPool = descriptorPool;
        init_info.Subpass        = 1;
        init_info.RenderPass     = renderpass;
        init_info.MinImageCount  = swapchain.surface_caps.minImageCount;  //2
        init_info.ImageCount     = swapchain.info.minImageCount;          //3
        init_info.MSAASamples    = VK_SAMPLE_COUNT_1_BIT;
        ImGui_ImplVulkan_Init(&init_info);
        ImGui_ImplVulkan_CreateFontsTexture();
        //-----------
    }

    void Destroy() {
        // ImGui Cleanup
        VKERRCHECK(vkDeviceWaitIdle(device));
        ImGui_ImplVulkan_Shutdown();
        ImGui_ImplvkWindow_Shutdown();
        ImGui::DestroyContext();
        if(!!descriptorPool) vkDestroyDescriptorPool(device, descriptorPool, NULL);
    }

    bool OnMouseEvent(eAction action, int16_t x, int16_t y, uint8_t btn) {
        static uint8_t btns = 0;
        if(action==eDOWN)  btns |= 1<<(btn-1);
        if(action==eUP  )  btns &= !(1<<(btn-1));
        ImGui_ImplvkWindow_UpdateMouse(btns, x, y);
        if(btn == 4) ImGui_ImplvkWindow_ScrollWheel(window, 0, 1.f);
        if(btn == 5) ImGui_ImplvkWindow_ScrollWheel(window, 0,-1.f);
        ImGuiIO& io = ImGui::GetIO();
        return io.WantCaptureMouse;  // return true if imgui handled the mouse event
    }

    bool HandleMouse() { // dont use (no mousewheel)
        int16_t mx, my;
        window->GetMousePos(mx, my);
        uint8_t btns = 0;
        if(window->GetBtnState(1)) btns+=1;
        if(window->GetBtnState(2)) btns+=2;
        if(window->GetBtnState(3)) btns+=4;
        ImGui_ImplvkWindow_UpdateMouse(btns, mx, my);
        ImGuiIO& io = ImGui::GetIO();
        return io.WantCaptureMouse;  // return true if imgui handled the mouse event
    }

    void NewFrame() {
        ImGui_ImplVulkan_NewFrame();
        ImGui_ImplvkWindow_NewFrame();
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
