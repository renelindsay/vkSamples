#include "OnScreen.h"
#include "CSkybox.h"
#include "Mesh.h"

void OnScreen::Init(CQueue& present_queue, CQueue& graphics_queue, VkSurfaceKHR surface) {
    auto& gpu   = graphics_queue.gpu;
    auto& device= graphics_queue.device;

#define TWOPASS
#ifdef  TWOPASS
    //--- Renderpass ---
    VkFormat present_fmt = gpu.FindSurfaceFormat(surface,{VK_FORMAT_R8G8B8A8_SRGB, VK_FORMAT_B8G8R8A8_SRGB});
    //VkFormat color_fmt = VK_FORMAT_R32G32B32A32_SFLOAT;
    VkFormat color_fmt = VK_FORMAT_R16G16B16A16_SFLOAT;

    VkFormat depth_fmt = gpu.FindDepthFormat();
    renderpass.Init(device, 2);  // 2 subpasses
    auto& subpass0 = renderpass.subpasses[0];
    auto& subpass1 = renderpass.subpasses[1];
    uint32_t depth_att   = renderpass.NewDepthAttachment  (depth_fmt, samples);
    uint32_t color_att   = renderpass.NewColorAttachment  (color_fmt, samples);
    uint32_t present_att = renderpass.NewPresentAttachment(present_fmt);
    subpass0.AddColorAttachment(color_att);
    subpass0.AddDepthAttachment(depth_att);
    subpass1.AddInputAttachment(color_att);
    subpass1.AddColorAttachment(present_att);
    renderpass.AddSubpassDependency(0,1);
    //-------------------
#else
    //--- Renderpass ---
    //VkFormat color_fmt = gpu.FindSurfaceFormat(surface,{VK_FORMAT_R8G8B8A8_UNORM, VK_FORMAT_B8G8R8A8_UNORM});   // linear
    VkFormat color_fmt = gpu.FindSurfaceFormat(surface,{VK_FORMAT_R8G8B8A8_SRGB,  VK_FORMAT_B8G8R8A8_SRGB});    // gamma (brighter)
    VkFormat depth_fmt = gpu.FindDepthFormat();
    renderpass.Init(device, 1);
    VkClearColorValue clear_col = {0.0f, 0.0f, 0.0f, 1.0f};
    uint32_t color_att = renderpass.NewPresentAttachment(color_fmt, samples, clear_col);
    uint32_t depth_att = renderpass.NewDepthAttachment  (depth_fmt, samples);
    auto& subpass0 = renderpass.subpasses[0];
    subpass0.AddColorAttachment(color_att);
    subpass0.AddDepthAttachment(depth_att);
    //-------------------
#endif
    renderpass.Print();

    //--- Swapchain ---
    swapchain.Init(renderpass, &present_queue, &graphics_queue);
    //swapchain.SetFramebufferCount(3);  // use triple-buffering
    //swapchain.PresentMode(VK_PRESENT_MODE_IMMEDIATE_KHR);
    swapchain.Print();
    //-----------------

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

#ifdef TWOPASS
    //---- Subpass 1 ----
    printf("\nSubpass 1:\n");
    pipeline_sub1.Init(renderpass, 1);
    pipeline_sub1.shader.MaxDescriptorSets(4);
    pipeline_sub1.shader.LoadVertShader("shaders/spirv/sub1_vert.spv");
    pipeline_sub1.shader.LoadFragShader("shaders/spirv/sub1_frag.spv");
    pipeline_sub1.CreateGraphicsPipeline();
    //-------------------
#endif
}

void OnScreen::Bind(CCamera& camera) {
    this->camera = &camera;
    CObject& root = camera.GetRoot();

    pbr_pipeline.shader.Bind("camera", camera.cam_ubo);
    sky_pipeline.shader.Bind("camera", camera.cam_ubo);

    root.recurse([&](CObject& node) {
        if(node.hitGroup==0) ((CBox& )node).pipeline = &sky_pipeline;  // MISS
        if(node.hitGroup==1) ((CMesh&)node).pipeline = &pbr_pipeline;  // HIT
    });
}

void OnScreen::Render() {
    CObject& root = camera->GetRoot();
    root.Transform_nodes();

    VkExtent2D ext  = swapchain.GetExtent();
    float w = (float)ext.width;
    float h = (float)ext.height;
    VkRect2D   scissor = {{0, 0}, ext};
    VkViewport viewport = {0, 0, w, h, 0, 1};

    float aspect = w / h;
    camera->SetPerspective(aspect, 40.f, 0.1f, 1000);
    //camera->flags = window.flags;

    VkCommandBuffer cmd = swapchain.BeginFrame();
        VkFence fence = swapchain.CurrBuffer().fence;
        vkCmdSetViewport(cmd, 0, 1, &viewport);
        vkCmdSetScissor (cmd, 0, 1, &scissor);
        camera->Apply(fence);
        root.Draw_nodes(cmd);
#ifdef TWOPASS
        vkCmdNextSubpass(cmd, VK_SUBPASS_CONTENTS_INLINE);
        pipeline_sub1.Bind(cmd, sub1_DS);
        vkCmdDraw(cmd, 3, 1, 0, 0);
#endif

    swapchain.EndFrame();
    //swapchain.ReadImage().Save("screenshot.png");
}
