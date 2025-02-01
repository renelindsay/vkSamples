#include "argparse.hpp"
#include "vkWindow.h"
#include "CDevices.h"
#include "CRenderpass.h"
#include "Swapchain.h"
#include "CPipeline.h"
#include "Buffers.h"
#include "CImage.h"
#include "matrix.h"
#include "CShader.h"

float dx = 0.1f;
float dy = 0.2f;
float scale = 1.f;

//-- EVENT HANDLERS --
class MainWindow : public vkWindow {
    using vkWindow::vkWindow;
    float mx = 0;
    float my = 0;

  public:
    //  window resized
    void OnResizeEvent(uint16_t width, uint16_t height) {
        printf("OnResizeEvent: %d x %d\n", width, height);
    }
    //  change spin with arrow keys
    void OnKeyEvent(eAction action, eKeycode keycode) {
        if(action==eDOWN) {
            if(keycode == KEY_Left ) dy+=0.1f;
            if(keycode == KEY_Right) dy-=0.1f;
            if(keycode == KEY_Up   ) dx+=0.1f;
            if(keycode == KEY_Down ) dx-=0.1f;
        }
    }

    //  change spin with mouse drag
    void OnMouseEvent(eAction action, int16_t x, int16_t y, uint8_t btn) {
        if(action==eMOVE && btn==1) { dy = mx - x;  dx = my - y;}
        mx = x; my = y;

        // -- Mouse wheel to zoom --
        if(action==eDOWN && btn==4) scale /= 1.1f;  //mouse wheel fwd
        if(action==eDOWN && btn==5) scale *= 1.1f;  //mouse wheel back
    }
    //  change spin with touch screen
    void OnTouchEvent(eAction action, float x, float y, uint8_t id) {
        if(id==0) {
            if(action==eMOVE && id==0) { dy = mx - x;  dx = my - y; }
            mx = x; my = y;
        }
    }
};

int main(int argc, char *argv[]) {
    int gpuid = 0;
#ifndef ANDROID
    argparse::ArgumentParser parser("cube", "0.1");
    parser.add_argument("-g", "--gpu").help("Select GPU number to use").scan<'i', int>().default_value(0);
    parser.parse_args(argc, argv);
    gpuid = parser.get<int>("gpu");
#endif

    setvbuf(stdout, NULL, _IONBF, 0);                           // Prevent printf buffering in QtCreator
    CInstance instance(true);                                   // Create a Vulkan Instance
    instance.DebugReport.SetFlags(14);                          // Error+Perf+Warning flags
    MainWindow window;                                          // Create a Vulkan window
    window.SetTitle("Example: 02_Cube");                        // Set the window title
    window.SetWinSize(640, 480);                                // Set the window size (Desktop)
    window.SetWinPos(0, 0);                                     // Set the window position to top-left
    VkSurfaceKHR surface = window.CreateVkSurface(instance);    // Create the Vulkan surface

    CPhysicalDevices gpus(instance);                            // Enumerate GPUs, and their properties
    //CPhysicalDevice *gpu = gpus.FindPresentable(surface);       // Find which GPU, can present to the given surface.
    CPhysicalDevice *gpu = &gpus[gpuid];

    gpus.Print();        // List the available GPUs.
    if (!gpu) return 0;  // Exit if no devices can present to the given surface.


    //gpu->extensions.Add("VK_KHR_sampler_ycbcr_conversion");
    //gpu->enable_features_11.samplerYcbcrConversion = true;

    gpu->enable_features.sampleRateShading = true;
    //gpu->enable_features.samplerAnisotropy = true;
    //gpu->enable_features.fillModeNonSolid  = true;
    //gpu->enable_features.wideLines         = true;
    //gpu->enable_features.shaderFloat64     = true;
    //gpu->enable_features.shaderStorageImageMultisample = true;

    //--- Device and Queues ---
    CDevice device(*gpu);                                              // Create Logical device on selected gpu
    CQueue* present_queue  = device.AddQueue(VK_QUEUE_GRAPHICS_BIT, surface);  // Create a graphics + present-queue
    CQueue* graphics_queue = present_queue;                                    // If possible use same queue for both
    if(!present_queue) {                                               // If not, create separate queues
        present_queue  = device.AddQueue(0, surface);                  // Create present-queue
        graphics_queue = device.AddQueue(VK_QUEUE_GRAPHICS_BIT);       // Create graphics queue
    }
    device.Create();                                                   // Create Logical device on selected gpu
    //-------------------------

    //---- Allocator ----
    CAllocator allocator;
    allocator.Init(instance, *graphics_queue);
    //allocator.maxAnisotropy = 16.f;
    //allocator.useRTX = hasRTX;
    //-------------------

    //--- Renderpass ---
    VkFormat color_fmt = gpu->FindSurfaceFormat(surface, {VK_FORMAT_R8G8B8A8_SRGB, VK_FORMAT_B8G8R8A8_SRGB});
    VkFormat depth_fmt = gpu->FindDepthFormat();

    CRenderpass renderpass(device);
    uint32_t depth_att = renderpass.NewDepthAttachment  (depth_fmt, VK_SAMPLE_COUNT_4_BIT);
    uint32_t color_att = renderpass.NewPresentAttachment(color_fmt, VK_SAMPLE_COUNT_4_BIT);
    auto& subpass0 = renderpass.subpasses[0];
    subpass0.AddColorAttachment(color_att);
    subpass0.AddDepthAttachment(depth_att);

    renderpass.Print();
    //-------------------
    //--- Swapchain ---
    Swapchain swapchain(renderpass, present_queue, graphics_queue);
    swapchain.SetFramebufferCount(3);  // use tripple-buffering
    swapchain.Print();
    //-----------------

    //--- Buffers ---
    struct Vertex {vec3 pos; vec2 tc;};
    const std::vector<Vertex> vertices = {
        //front
        {{-0.5f,-0.5f, 0.5f}, {1.0f, 0.0f}},
        {{ 0.5f,-0.5f, 0.5f}, {0.0f, 0.0f}},
        {{ 0.5f, 0.5f, 0.5f}, {0.0f, 1.0f}},
        {{-0.5f, 0.5f, 0.5f}, {1.0f, 1.0f}},
        //back
        {{ 0.5f,-0.5f,-0.5f}, {1.0f, 0.0f}},
        {{-0.5f,-0.5f,-0.5f}, {0.0f, 0.0f}},
        {{-0.5f, 0.5f,-0.5f}, {0.0f, 1.0f}},
        {{ 0.5f, 0.5f,-0.5f}, {1.0f, 1.0f}},
        //left
        {{-0.5f,-0.5f,-0.5f}, {1.0f, 0.0f}},
        {{-0.5f,-0.5f, 0.5f}, {0.0f, 0.0f}},
        {{-0.5f, 0.5f, 0.5f}, {0.0f, 1.0f}},
        {{-0.5f, 0.5f,-0.5f}, {1.0f, 1.0f}},
        //right
        {{ 0.5f,-0.5f, 0.5f}, {1.0f, 0.0f}},
        {{ 0.5f,-0.5f,-0.5f}, {0.0f, 0.0f}},
        {{ 0.5f, 0.5f,-0.5f}, {0.0f, 1.0f}},
        {{ 0.5f, 0.5f, 0.5f}, {1.0f, 1.0f}},
        //top
        {{-0.5f,-0.5f,-0.5f}, {1.0f, 0.0f}},
        {{ 0.5f,-0.5f,-0.5f}, {0.0f, 0.0f}},
        {{ 0.5f,-0.5f, 0.5f}, {0.0f, 1.0f}},
        {{-0.5f,-0.5f, 0.5f}, {1.0f, 1.0f}},
        //bottom
        {{-0.5f, 0.5f, 0.5f}, {1.0f, 0.0f}},
        {{ 0.5f, 0.5f, 0.5f}, {0.0f, 0.0f}},
        {{ 0.5f, 0.5f,-0.5f}, {0.0f, 1.0f}},
        {{-0.5f, 0.5f,-0.5f}, {1.0f, 1.0f}},
    };

    const std::vector<uint16_t> index = {
        0, 1, 2,  2, 3, 0, 
        4, 5, 6,  6, 7, 4,
        8, 9,10, 10,11, 8,
       12,13,14, 14,15,12,
       16,17,18, 18,19,16,
       20,21,22, 22,23,20
    };

    struct Uniforms {
        mat4 model;
        mat4 view;
        mat4 proj;
    } uniforms;

    uniforms.view.Translate(0,0,-4);

    // Vertex Buffer Object
    VBO vbo;                                                                      // Create vertex buffer
    vbo.Data((void*)vertices.data(), (uint32_t)vertices.size(), sizeof(Vertex));  // load vertex data
    printf("VBO created\n");
    //---------------------

    // Index Buffer Object
    IBO ibo;
    ibo.Data(index.data(), (uint32_t)index.size());
    printf("IBO created\n");
    //---------------------

    // Uniform Buffer Object
    UBO ubo;
    ubo.Allocate(sizeof(uniforms));
    printf("UBO created\n");
    //----------------------

    // Textures
    CImage img("Vulkan.png");  //  Load image from file
    CvkImage vkImg = img;      //  Copy image to GPU memory
    //CvkImage vkImg(img, VK_FORMAT_R8G8B8A8_SRGB, true);
    // ---------

    //img.sRGBtoUNORM();
    //CvkImage vkImg(img, VK_FORMAT_R8_UNORM, true);
    //CvkImage vkImg(img, VK_FORMAT_R5G6B5_UNORM_PACK16, true);
    //CvkImage vkImg(img, VK_FORMAT_BC1_RGB_UNORM_BLOCK, true);
    //CvkImage vkImg(img, VK_FORMAT_BC1_RGBA_SRGB_BLOCK, true);
    //CvkImage vkImg(img, VK_FORMAT_BC4_UNORM_BLOCK, true);
    //CvkImage vkImg(img, VK_FORMAT_G8B8G8R8_422_UNORM, false);
    // ---------

    //--
    CPipeline pipeline(renderpass);
    pipeline.shader.MaxDescriptorSets(32);
    pipeline.shader.LoadVertShader("shaders/spirv/shader_vert.spv");
    pipeline.shader.LoadFragShader("shaders/spirv/shader_frag.spv");
    pipeline.shader.Bind("ubo", ubo);
    pipeline.shader.Bind("texSampler", vkImg);


    //VkDescriptorSet   descriptorSet  = pipeline.shader.CreateDescriptorSet();
    //VkPipelineLayout  pipelineLayout = pipeline.shader.GetPipelineLayout();

    //VkDescriptorSets ds;
    //pipeline.shader.UpdateDescriptorSet(ds);

    VkDescriptorSets ds = pipeline.shader.CreateDescriptorSets();

    pipeline.CreateGraphicsPipeline();
    printf("Pipeline created\n");
    //--


    //--- Main Loop ---
    while (window.PollEvents()) {  // Main event loop, runs until window is closed.
        VkExtent2D ext  = swapchain.GetExtent();
        VkRect2D   scissor = {{0, 0}, ext};
        VkViewport viewport = {0, 0, (float)ext.width, (float)ext.height, 0, 1};

        float aspect = (float)ext.width/(float)ext.height;
        uniforms.proj.SetPerspective(aspect, 40.f, 1, 1000);

        uniforms.model.TransposeSelf();
        uniforms.model.RotateX(-dx/2.f);
        uniforms.model.RotateY(-dy/2.f);
        uniforms.model.TransposeSelf();

        float curr = uniforms.model.xAxis().length();
        uniforms.model.Scale(scale / curr);

        ubo.Update(&uniforms);

        VkCommandBuffer cmd_buf = swapchain.BeginFrame();
            vkCmdSetViewport(cmd_buf,0,1, &viewport);
            vkCmdSetScissor(cmd_buf,0,1, &scissor);

            pipeline.Bind(cmd_buf, ds);
            //vkCmdBindPipeline(cmd_buf, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
            //vkCmdBindDescriptorSets(cmd_buf, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, &descriptorSet, 0, nullptr);

            //VkBuffer vertexBuffers[] = {vbo};
            //VkDeviceSize offsets[] = {0};
            //vkCmdBindVertexBuffers(cmd_buf, 0, 1, vertexBuffers, offsets);

            vkCmdBindVertexBuffer(cmd_buf, vbo);
            vkCmdBindIndexBuffer(cmd_buf, ibo, 0, VK_INDEX_TYPE_UINT16);
            vkCmdDrawIndexed(cmd_buf, ibo.Count(), 1, 0, 0, 0);

        swapchain.EndFrame();
    }
    //-----------------
    return 0;
}
