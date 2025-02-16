#ifndef ONSCREEN_H
#define ONSCREEN_H

#include "CDevices.h"
#include "CRenderpass.h"
#include "CPipeline.h"
#include "Swapchain.h"
#include "CCamera.h"

//#define ENABLE_IMGUI
#ifdef  ENABLE_IMGUI
#include "DearImGui.h"
#endif

class OnScreen {
    CRenderpass renderpass;
    CPipeline   pbr_pipeline;
    CPipeline   sky_pipeline;
    CPipeline   pipeline_sub1;
    VkDescriptorSets sub1_DS;
    CCamera*  camera = 0;
public:
    Swapchain swapchain;
#ifdef  ENABLE_IMGUI
    DearImGui im;
#endif
    void Init(CQueue& present_queue, CQueue& graphics_queue, VkSurfaceKHR surface);
    void Bind(CCamera& camera);
    void Render();
};

#endif
