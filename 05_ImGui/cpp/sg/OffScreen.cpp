#include "OffScreen.h"
#include "CSkybox.h"
#include "Mesh.h"

void OffScreen::Init(CQueue& queue) {
    //--- Renderpass ---
    //VkFormat color_fmt = VK_FORMAT_R8G8B8A8_UNORM;
    //VkFormat color_fmt = VK_FORMAT_B8G8R8A8_UNORM;
    VkFormat color_fmt = VK_FORMAT_B8G8R8A8_SRGB;
    VkFormat depth_fmt = queue.gpu.FindDepthFormat();
    renderpass.Init(queue.device, 1);
    uint32_t depth_att = renderpass.NewDepthAttachment  (depth_fmt, VK_SAMPLE_COUNT_8_BIT);
    uint32_t color_att = renderpass.NewPresentAttachment(color_fmt, VK_SAMPLE_COUNT_8_BIT,{1.f,0,0,1.f});
    auto& subpass0 = renderpass.subpasses[0];
    subpass0.AddColorAttachment(color_att);
    subpass0.AddDepthAttachment(depth_att);
    renderpass.Print();
    //------------------

    //--- FBO ---
    fbo.Init(renderpass, &queue);
    fbo.SetExtent(512, 512);
    //-----------

    //--- Pipelines ---
    pbr_pipeline.Init(renderpass, 0);
    pbr_pipeline.shader.MaxDescriptorSets(32);
    pbr_pipeline.shader.LoadVertShader("shaders/spirv/pbr_vert.spv");
    pbr_pipeline.shader.LoadFragShader("shaders/spirv/pbr_frag.spv");
    pbr_pipeline.CreateGraphicsPipeline();

    sky_pipeline.Init(renderpass, 0);
    sky_pipeline.shader.MaxDescriptorSets(3);
    sky_pipeline.shader.LoadVertShader("shaders/spirv/sky_vert.spv");
    sky_pipeline.shader.LoadFragShader("shaders/spirv/sky_frag.spv");
    sky_pipeline.depthStencilState.depthWriteEnable = VK_FALSE;          // Skybox does not modify depth
    sky_pipeline.rasterizer.depthClampEnable = VK_TRUE;                  // Dont clip skybox on farplane
    sky_pipeline.CreateGraphicsPipeline();
    //-----------------
}

void OffScreen::Bind(CCamera& camera) {
    this->camera = &camera;
    CObject& root = camera.GetRoot();

    pbr_pipeline.shader.Bind("camera", camera.cam_ubo);
    sky_pipeline.shader.Bind("camera", camera.cam_ubo);

    root.recurse([&](CObject& node) {
        if(node.hitGroup==0) ((CBox& )node).pipeline = &sky_pipeline;  // MISS
        if(node.hitGroup==1) ((CMesh&)node).pipeline = &pbr_pipeline;  // HIT
    });
}

void OffScreen::Render() {
    CObject& root = camera->GetRoot();

    VkExtent2D ext  = fbo.GetExtent();
    float w = (float)ext.width;
    float h = (float)ext.height;
    VkRect2D   scissor = {{0, 0}, ext};
    VkViewport viewport = {0, 0, w, h, 0, 1};

    float aspect = w / h;
    camera->SetPerspective(aspect, 40.f, 0.1f, 1000);
    //camera->flags = window.flags;

    root.Transform_nodes();
    VkCommandBuffer cmd = fbo.BeginFrame();
        VkFence fence = fbo.CurrBuffer().fence;
        vkCmdSetViewport(cmd, 0, 1, &viewport);
        vkCmdSetScissor (cmd, 0, 1, &scissor);
        camera->Apply(fence);
        root.Draw_nodes(cmd);
    fbo.EndFrame();

    //fbo.ReadImage().Save("fbo.png");
}
