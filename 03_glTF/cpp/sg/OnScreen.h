#ifndef ONSCREEN_H
#define ONSCREEN_H

#include "CDevices.h"
#include "CRenderpass.h"
#include "Swapchain.h"
#include "CObject.h"
#include "CCamera.h"
#include "CSkybox.h"
#include "glTF.h"

class OnScreen {
    CRenderpass renderpass;
    CPipeline   pbr_pipeline;
    CPipeline   sky_pipeline;
    CPipeline   pipeline_sub1;
    VkDescriptorSets  sub1_DS;

public:
    Swapchain swapchain;
    CCamera*  camera = 0;

    void Init(CQueue& present_queue, CQueue& graphics_queue, VkSurfaceKHR surface);
    void Bind(CCamera& camera);
    void Render();
};

#endif
