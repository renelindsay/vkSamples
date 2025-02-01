#ifndef OFFSCREEN_H
#define OFFSCREEN_H

#include "CDevices.h"
#include "CRenderpass.h"
#include "Swapchain.h"
#include "CObject.h"
#include "CCamera.h"
#include "CSkybox.h"
#include "glTF.h"

class OffScreen {
    CRenderpass renderpass;
    CPipeline   pbr_pipeline;
    CPipeline   sky_pipeline;
public:
    FBO         fbo;
    CCamera*    camera = 0;

    void Init(CQueue& queue);
    void Bind(CCamera& camera);
    void Render();
};

#endif
