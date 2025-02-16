/*
// Rene Lindsay 2019
*
*  This unit wraps the swapchain, for rendering to a window vkSurface.
*
*  WARNING: This unit is a work in progress.
*  Interfaces are experimental and likely to change.
*
*  Swapchain requires a renderpass, a graphics queue and a present queue.
*  The the present CQueue must be linked to the window surface.
*  The CRenderpass requires at least one color attachment, and optionally a depth attachment.
*
*  Use the PresentMode() function to select vsync behaviour (FIFO / MAILBOX / ...)
*  Use the SetFramebufferCount() to select double or triple buffering. (default is 2: double-buffering)
*
*  PRESENTING:
*  Call BeginFrame() to acquire the next frame's command buffer.
*  Record vkCmd* commands, using the returned command buffer.
*  Call EndFrame() to execute and Present image, when done.
*
*  When using subpass rendering, Swapchain will create and manage
*  additional color attachments as intermediate subpass render targets.
*
*  For offscreen rendering, where the final target is not a vkSurface,
*  use the FBO class instead.
*/

#ifndef CSWAPCHAIN_H
#define CSWAPCHAIN_H

//#include "vkWindow.h"
#include "CDevices.h"
#include "CRenderpass.h"
#include "Buffers.h"
#include "FBO.h"

#ifdef ANDROID
#define IS_ANDROID true  // ANDROID: default to power-save (limit to 60fps)
#else
#define IS_ANDROID false // PC: default to low-latency (no fps limit)
#endif

class Swapchain : public FBO {
    VkQueue        present_queue;
    VkSurfaceKHR   surface;
    VkSwapchainKHR swapchain;
    bool resized = false;  // TODO: Remove?

    void Clear();
    void Init(const CQueue* present_queue, const CQueue* graphics_queue=0);
    bool SetExtent();
    void Apply();
public:
    VkSurfaceCapabilitiesKHR surface_caps{};
    VkSwapchainCreateInfoKHR info{};

    Swapchain(CRenderpass& renderpass, const CQueue* present_queue, const CQueue* graphics_queue);
    Swapchain(){};
    ~Swapchain();
    void Init(CRenderpass& renderpass, const CQueue* present_queue, const CQueue* graphics_queue);

    bool PresentMode(bool no_tearing, bool powersave = IS_ANDROID);            // ANDROID: default to power-save mode (limit to 60fps)
    bool PresentMode(VkPresentModeKHR preferred_mode);                         // If mode is not available, returns false and uses FIFO.
    bool SetFramebufferCount(uint32_t image_count = 2);                        // 2=double-buffer 3=triple-buffer. returns true on success
    bool Resized() {bool flag = resized; resized = false; return flag;}        // returns true is swapchain resized since last call

    VkExtent2D GetExtent(){return info.imageExtent;}
    void Print();

    //-----Simple interface-----
    using FBO::BeginFrame;       // Get next cmd buffer and start recording commands
    using FBO::EndFrame;         // End the renderpass and present
    //--------------------------
    //----Flexible interface----
    FrameBuffer& AcquireNext();  // Acquires next frame buffer
    using FBO::BeginCmd;         // Begin recording command buffer
    using FBO::BeginRenderpass;  // Begin the renderpass
    //...                        // vkCmd... commands go here
    using FBO::EndRenderpass;    // End the renderpass
    using FBO::EndCmd;           // End recording the command buffer
    void Submit();               // Swaps frame buffers, to show the rendered image
    //--------------------------
    CImage& ReadImage();          // Copy last rendered frame to host memory

    operator VkSwapchainKHR() {return swapchain; }
};


#endif


