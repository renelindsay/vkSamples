#pragma warning(disable:4996)  // for fopen

#include "argparse.hpp"
#include "vkWindow.h"
#include "OnScreen.h"
#include "OffScreen.h"
#include "Scene.h"
#ifdef ENABLE_IMGUI
#include "DearImGui.h"
#endif

//-- EVENT HANDLERS --
class MainWindow : public vkWindow {
    using vkWindow::vkWindow;

    float m_mx = 0;
    float m_my = 0;
    float m_dist = 0;

    float m_down_x = 0;
    float m_down_y = 0;
    bool  m_keyboardVisible = false;
  public:
    Scene* scene;
#ifdef ENABLE_IMGUI
    DearImGui* im;
#endif
    uint32_t flags  = 0;

    void OnResizeEvent(uint16_t width, uint16_t height) {
        printf("OnResizeEvent: %d x %d\n", width, height);
    }

    void OnKeyEvent(eAction action, eKeycode keycode) {
        if(action==eDOWN) {
            if(keycode == KEY_1) flags = 1;  // diffuse light
            if(keycode == KEY_2) flags = 2;  // specular light
            if(keycode == KEY_3) flags = 3;  // albedo texture
            if(keycode == KEY_4) flags = 4;  // normals (no normal map)
            if(keycode == KEY_5) flags = 5;  // normals (with normal map)
            if(keycode == KEY_6) flags = 6;  // Ambient occlusion
            if(keycode == KEY_7) flags = 7;  // roughness(green) + metalness(blue)
            if(keycode == KEY_8) flags = 8;  // fresnel effect
            if(keycode == KEY_9) flags = 9;  // final render
            if(keycode == KEY_0) flags = 0;  // final render
        }
        scene->camera.flags = flags;
    }

    // mouse drag
    void OnMouseEvent(eAction action, int16_t x, int16_t y, uint8_t btn) {
#ifdef ENABLE_IMGUI
        if(im->OnMouseEvent(action, x, y, btn)) return;
#endif
        if(action==eMOVE && btn==1) { 
            float dy = x - m_mx;
            float dx = y - m_my;
            scene->camX.matrix.RotateY(dy/8);
            scene->camY.matrix.RotateX(dx/8);
        }
        m_mx = x; m_my = y;

        // -- Mouse wheel to zoom --
        if(scene->camera.matrix.position().z <=0.01) scene->camera.matrix.position().z = 0.01; // clamp min distance
        if(action==eDOWN && btn==4) scene->camera.matrix.position().z /= 1.1f;  //mouse wheel fwd
        if(action==eDOWN && btn==5) scene->camera.matrix.position().z *= 1.1f;  //mouse wheel back
    }
    //  touch screen
    void OnTouchEvent(eAction action, float x, float y, uint8_t id) {
#ifdef ENABLE_IMGUI
        if(im->OnTouchEvent(action,x,y,id)) return;
#endif
        const char *type[] {"up  ", "down", "move"};
        printf("Touch: %s %f x %f id:%d\n", type[action], x, y, id); 

        float dy = x - m_mx;
        float dx = y - m_my;

        if(id==0) {
            if(action==eMOVE && m_dist==0) {
                scene->camX.matrix.RotateY(dy/4);
                scene->camY.matrix.RotateX(dx/4);
            }
            m_mx = x; m_my = y;
        }

        // -- Two-finger pinch to zoom --
        if(id==1) {
            if(action==eDOWN) {m_dist = sqrt(dx*dx + dy*dy);}
            if(action==eUP)   {m_dist = 0;}

            if(action==eMOVE) {
                float new_dist = sqrt(dx*dx + dy*dy);
                float delta = (new_dist / m_dist);
                scene->camera.matrix.position().z /= delta;
                m_dist = new_dist;
            }
        }

        // -- Single-click to show/hide Android keyboard --
        if(id==0) {
            if(action == eDOWN) {
                m_down_x = x;
                m_down_y = y;
            }
            if(action == eUP) {
                float dx = std::abs( m_down_x - x);
                float dy = std::abs( m_down_y - y);
                if( dx + dy < 10.f) ShowKeyboard(!m_keyboardVisible);
                m_keyboardVisible = !m_keyboardVisible;
            }
        }
    }
};

int main(int argc, char *argv[]) {
    int gpuid = 0;
#ifndef ANDROID
    argparse::ArgumentParser parser("vkRay", "0.1");
    parser.add_argument("-g", "--gpu").help("Select GPU number to use").scan<'i', int>().default_value(0);
    parser.parse_args(argc, argv);
    gpuid = parser.get<int>("gpu");
#endif

    setvbuf(stdout, NULL, _IONBF, 0);                           // Prevent printf buffering in QtCreator
    CInstance instance(true);                                   // Create a Vulkan Instance
    instance.DebugReport.SetFlags(14);                          // Error+Perf+Warning flags
    MainWindow window;                                          // Create a Vulkan window
    window.SetTitle("05_imGui");                                // Set the window title
    window.SetWinSize(640, 480);                                // Set the window size (Desktop)
    window.SetWinPos(0, 0);                                     // Set the window position to top-left
    VkSurfaceKHR surface = window.CreateVkSurface(instance);    // Create the Vulkan surface

    CPhysicalDevices gpus(instance);                            // Enumerate GPUs, and their properties
    //CPhysicalDevice* gpu = gpus.FindPresentable(surface);       // Find which GPU, can present to the given surface.
    CPhysicalDevice* gpu = &gpus[gpuid];

    gpus.Print();        // List the available GPUs.
    if (!gpu) return 0;  // Exit if no devices can present to the given surface.

    //gpu->enable_features.samplerAnisotropy = VK_TRUE;
    gpu->enable_features.sampleRateShading = VK_TRUE;
    gpu->extensions.Add("VK_KHR_buffer_device_address");

    //--- Device and Queues ---
    CDevice device(*gpu);                                                      // Create Logical device on selected gpu
    CQueue* present_queue  = device.AddQueue(VK_QUEUE_GRAPHICS_BIT, surface);  // Create a graphics + present-queue
    CQueue* graphics_queue = present_queue;                                    // If possible use same queue for both
    if(!present_queue) {                                                       // If not, create separate queues
        present_queue  = device.AddQueue(0, surface);                          // Create present-queue
        graphics_queue = device.AddQueue(VK_QUEUE_GRAPHICS_BIT);               // Create graphics queue
    }
    device.Create();                                                           // Create Logical device on selected gpu
    //-------------------------

    //---- Allocator ----
    CAllocator allocator;
    allocator.Init(instance, *graphics_queue);
    //allocator.maxAnisotropy = 16.f;
    allocator.pack_normals = true;
    //-------------------

    Scene scene;
    scene.Init();
    window.scene = &scene;

    //----Onscreen----
    OnScreen onscreen;
#ifdef ENABLE_IMGUI
    //--for imgui--
    onscreen.im.instance = instance;
    onscreen.im.window = &window;
    window.im = &onscreen.im;
    //-------------
#endif
    onscreen.Init(*present_queue, *graphics_queue, surface);
    onscreen.Bind(scene.camera);
    //----------------

    //----OffScreen----
    OffScreen offscreen;
    offscreen.Init(*graphics_queue);
    //-----------------

    //----Init scene nodes----
    scene.root.Init_nodes();
    scene.root.Print();             // print scene graph structure
    //------------------------

    //==============================================

    //--- Main Loop ---
    while (window.ProcessEvents()) {
 #ifdef ENABLE_IMGUI
        window.im->NewFrame();
        //window.im->HandleMouse();

        static bool show_demo_window = true;
        if (show_demo_window) {
            ImGui::ShowDemoWindow(&show_demo_window);
            ImGuiWindow* demoWindow = ImGui::FindWindowByName("Dear ImGui Demo");
            if (demoWindow) demoWindow->Pos = ImVec2(10, 10);
        }
#endif
        onscreen.Bind(scene.camera);
        onscreen.Render();

        if(window.GetKeyState(KEY_S)) {  // 'S': save screenshot
            CvkImage& attachment = onscreen.swapchain.att_images[1];
            attachment.Read().Save("frame.png");
        }

        if(window.GetKeyState(KEY_F)) {  //'F': save image from FBO
            offscreen.Bind(scene.camera);
            offscreen.Render();
            CImage& img = offscreen.fbo.ReadImage();
            window.ShowImage(img, 512,512);
            img.BGRAtoRGBA();
            img.Save("fbo.png");
        }
    }
    //-----------------
    return 0;
}
