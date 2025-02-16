#ifndef ONSCREEN_H
#define ONSCREEN_H

#include "CDevices.h"
#include "CRenderpass.h"
#include "CPipeline.h"
#include "Swapchain.h"
#include "CCamera.h"

class OnScreen {
    CRenderpass renderpass;
    CPipeline   pbr_pipeline;
    CPipeline   sky_pipeline;
    CPipeline   pipeline_sub1;
    VkDescriptorSets sub1_DS;
    CCamera*  camera = 0;
public:
    VkSampleCountFlagBits samples = VK_SAMPLE_COUNT_1_BIT;
    Swapchain swapchain;
    void Init(CQueue& present_queue, CQueue& graphics_queue, VkSurfaceKHR surface);
    void Bind(CCamera& camera);
    void Render();
};

#endif
