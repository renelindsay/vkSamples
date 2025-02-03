#ifndef OFFSCREEN_H
#define OFFSCREEN_H

#include "CDevices.h"
#include "CRenderpass.h"
#include "CCamera.h"
#include "FBO.h"

class OffScreen {
    CRenderpass renderpass;
    CPipeline   pbr_pipeline;
    CPipeline   sky_pipeline;
    CCamera*    camera = 0;
public:
    FBO  fbo;
    void Init(CQueue& queue);
    void Bind(CCamera& camera);
    void Render();
};

#endif
