/*
 *  This unit wraps the frame buffer object, and is the base class for Swapchain.
 *  Use FBO to render to an off-screen color image target.
 *  For rendering to a window surface, use Swapcain instead.
 *
 *  FBO requires a renderpass and a graphics queue. (no present queue)
 *  For subpass rendering, FBO will allocate additional color attachments,
 *  for use as intermediate render targets.
 *
 *  Use SetFramebufferCount() to set the number of frame buffers. (default=2)
 *  Use SetExtent to set the width & height of the render target.
 *
 *  Simple interface:
 *    Call BeginFrame() to acquire the next frame and its command buffer.
 *    Record vkCmd* commands, using the returned command buffer.
 *    Call EndFrame() to submit the commands to the GPU for rendering.
 *
 * Flexible interface:
 *    The alternative flexible interface is more verbose, for fine-grained control.
 *
*/

#ifndef FBO_H
#define FBO_H

//#include "vkWindow.h"
#include "CDevices.h"
#include "CRenderpass.h"
#include "Buffers.h"

struct FrameBuffer {
    VkImage         image         = nullptr;
    VkImageView     view          = nullptr;
    VkExtent2D      extent        {};
    VkFramebuffer   framebuffer   = nullptr;
    VkCommandBuffer command_buffer= nullptr;
    VkSemaphore acquire_semaphore = nullptr;  // signaled when a new fb is acquired, and cmd-submit may begin (Swapchain only)
    VkSemaphore submit_semaphore  = nullptr;  // signaled that submit is complete and present may begin (Swapchain only)
    VkFence         fence         = nullptr;  // signaled when rendering to this FB is complete
};

class FBO {
    friend class Swapchain;
    VkPhysicalDevice  gpu           = nullptr;
    VkDevice          device        = nullptr;
    VkQueue           graphics_queue= nullptr;
    VkCommandPool     command_pool  = nullptr;
    CRenderpass*      renderpass    = nullptr;
    CDepthBuffer      depth_buffer;
    //CAllocator        export_allocator;

    VkFormat   format = VK_FORMAT_UNDEFINED;
    VkExtent2D extent = {256,256};

    std::vector<FrameBuffer> buffers;
    std::vector<CvkImage>    images;  // Render target (offscreen)

    uint32_t rendered_index = 0;      // index of last rendered image
    uint32_t acquired_index = 0;      // index of last acquired image
    bool is_acquired = false;

    void Clear();
    void Init(const CQueue* graphics_queue);
    void ResizeAttachments();
    void Apply();
public:
    std::vector<CvkImage> att_images;  // additional attachments, besides the depth and present buffers

    FBO(CRenderpass& renderpass, const CQueue* graphics_queue);
    FBO(){};
    ~FBO();
    void Init(CRenderpass& renderpass, const CQueue* graphics_queue);
    bool SetFramebufferCount(uint32_t count=2);       // 2=double-buffer 3=triple-buffer. returns true on success
    void SetExtent(uint32_t width, uint32_t height);  // resize FrameBuffer
    virtual VkExtent2D GetExtent(){ return extent;}

    FrameBuffer& CurrBuffer() {return buffers[acquired_index];}  // buffer being rendered
    FrameBuffer& PrevBuffer() {return buffers[rendered_index];}  // last rendered buffer

    //-----Simple interface-----
    VkCommandBuffer BeginFrame();        // Get next cmd buffer and start recording commands
    void EndFrame();                     // End the renderpass and present
    //--------------------------
    //----Flexible interface----
    virtual FrameBuffer& AcquireNext();  // Acquires next frame buffer
    VkCommandBuffer BeginCmd();          // Begin recording command buffer
    void BeginRenderpass();              // Begin the renderpass
    //...                                // vkCmd... commands go here
    void EndRenderpass();                // End the renderpass
    void EndCmd();                       // End recording the command buffer
    virtual void Submit();               // Swap frame buffers (no wait)
    void Wait();                         // Wait until submit is done
    //--------------------------
    CImage& ReadImage();                  // Copy last rendered frame to host memory
};

#endif
