// * Copyright (C) 2017 by Rene Lindsay

#include "Swapchain.h"
//#include "VkFormats.h"

//--------------------------Swapchain---------------------------
Swapchain::Swapchain(CRenderpass& renderpass, const CQueue* present_queue, const CQueue* graphics_queue) {
    Init(renderpass, present_queue, graphics_queue);
}

void Swapchain::Init(CRenderpass& renderpass, const CQueue* present_queue, const CQueue* graphics_queue) {
    //-- Check color attachment count --
    //int32_t color_cnt = (int32_t)renderpass.attachments.size();
    //if(renderpass.depth_format != VK_FORMAT_UNDEFINED) color_cnt -=1;
    //ASSERT(color_cnt == 1, "Swapchain may only have 1 color attachment. For offscreen rendering with MRT, use FBO instead.\n");
    //----------------------------------

    ASSERT(!this->renderpass, "Renderpass already selected.");
    this->renderpass = &renderpass;
    if(!graphics_queue) graphics_queue = present_queue;
    Init(present_queue, graphics_queue);
}

Swapchain::~Swapchain() {
    Clear();
    if (swapchain) vkDestroySwapchainKHR(device, swapchain, nullptr);  swapchain=nullptr;
    LOGI("Swapchain destroyed\n");
}

void Swapchain::Clear() {
    vkDeviceWaitIdle(device);
    for(auto& buf : buffers) {
        // ---Wait for fence, before destroying ---
        vkWaitForFences(device, 1, &buf.fence, VK_TRUE, UINT64_MAX);
        VKERRCHECK(vkGetFenceStatus(device, buf.fence));
        // ----------------------------------------

        vkDestroyFence      (device, buf.fence,             nullptr);  buf.fence      =nullptr;
        vkDestroyFramebuffer(device, buf.framebuffer,       nullptr);  buf.framebuffer=nullptr;
        vkDestroyImageView  (device, buf.view,              nullptr);  buf.view       =nullptr;
        vkDestroySemaphore  (device, buf.acquire_semaphore, nullptr);  buf.acquire_semaphore=nullptr;
        vkDestroySemaphore  (device, buf.submit_semaphore,  nullptr);  buf.submit_semaphore =nullptr;
    }
}

void Swapchain::Init(const CQueue* present_queue, const CQueue* graphics_queue) {
    this->gpu     = present_queue->gpu;
    this->surface = present_queue->surface;
    this->device  = present_queue->device;
    swapchain     = 0;
    is_acquired   = false;
    this->present_queue  = *present_queue;
    this->graphics_queue = *graphics_queue;
    this->command_pool = graphics_queue->CreateCommandPool();

    //--- surface caps ---
    VKERRCHECK(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(gpu, surface, &surface_caps));
    assert(surface_caps.supportedUsageFlags & VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT);
    //assert(surface_caps.supportedTransforms & surface_caps.currentTransform);
    assert(surface_caps.supportedTransforms & VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR);
    assert(surface_caps.supportedCompositeAlpha & (VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR | VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR));
    //--------------------

    info = {VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR};
    info.surface               = surface;
    //info.minImageCount         = 2; // double-buffer
    info.imageFormat           = renderpass->GetPresentFormat(); // renderpass->present_format;
    info.imageColorSpace       = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;

    //info.imageExtent           = {64, 64}; //extent;
    info.imageArrayLayers      = 1;  // 2 for stereo
    info.imageUsage            = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT
                               | VK_IMAGE_USAGE_TRANSFER_SRC_BIT
                               | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    info.imageSharingMode      = VK_SHARING_MODE_EXCLUSIVE;
    info.queueFamilyIndexCount = 0;
    info.pQueueFamilyIndices   = 0;
    info.preTransform          = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
    //info.compositeAlpha        = composite_alpha;
    info.presentMode           = VK_PRESENT_MODE_FIFO_KHR;
    info.clipped               = true;
    //info.oldSwapchain          = swapchain;
    
    if(present_queue->family != graphics_queue->family) {
        uint32_t families[2] = {present_queue->family, graphics_queue->family};
        info.imageSharingMode      = VK_SHARING_MODE_CONCURRENT;
        info.queueFamilyIndexCount = 2;
        info.pQueueFamilyIndices   = families;
    }
    
    info.compositeAlpha = (surface_caps.supportedCompositeAlpha & VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR) ?
                           VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR : VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    SetExtent();  // also initializes surface_caps

    auto* att = renderpass->GetDepthAttachment();
    if(att) depth_buffer.SetSize(extent, att->format, att->samples);

    SetFramebufferCount(2);
    Apply();
}

bool operator==(const VkExtent2D& lhs, const VkExtent2D& rhs) {
    return lhs.width == rhs.width && lhs.height == rhs.height;
}

void Swapchain::SetExtent() {  // Fit image extent to window size
    VKERRCHECK(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(gpu, surface, &surface_caps));
    VkExtent2D& curr = surface_caps.currentExtent;  // surface extent
    VkExtent2D& ext = info.imageExtent;             // swapchain extent
    if(ext == curr) return;
    //printf("swapchain: w=%d h=%d curr_w=%d curr_h=%d\n", ext.width, ext.height, curr.width, curr.height);
    ext = curr;
    auto clamp =[](uint32_t val, uint32_t min, uint32_t max){ return (val < min ? min : val > max ? max : val); };
    ext.width  = clamp(ext.width,  surface_caps.minImageExtent.width,  surface_caps.maxImageExtent.width);
    ext.height = clamp(ext.height, surface_caps.minImageExtent.height, surface_caps.maxImageExtent.height);
    extent = ext;
    resized = true;
     if (!!swapchain) Apply();
}

bool Swapchain::SetFramebufferCount(uint32_t image_count) {  // set number of framebuffers. (2 or 3). Return true on success.
    uint32_t count = std::max(image_count, surface_caps.minImageCount);                      //clamp to min limit
    if(surface_caps.maxImageCount > 0) count = std::min(count, surface_caps.maxImageCount);  //clamp to max limit
    info.minImageCount = count;
    if(count != image_count) LOGW("Swapchain using %d framebuffers, instead of %d.\n", count, image_count);
    if (!!swapchain) Apply();
    return (count == image_count);
}

// Returns the list of avialable present modes for this gpu + surface.
std::vector<VkPresentModeKHR> GetPresentModes(VkPhysicalDevice gpu, VkSurfaceKHR surface) {
    uint32_t count = 0;
    std::vector<VkPresentModeKHR> modes;
    vkGetPhysicalDeviceSurfacePresentModesKHR(gpu, surface, &count, nullptr);
    assert(count > 0);
    modes.resize(count);
    VKERRCHECK(vkGetPhysicalDeviceSurfacePresentModesKHR(gpu, surface, &count, modes.data()));
    return modes;
}

// ---------------------------- Present Mode ----------------------------
// no_tearing : TRUE = Wait for next vsync, to swap buffers.  FALSE = faster fps, but with tearing.
// powersave  : TRUE = Limit framerate to vsync (60 fps).     FALSE = lower latency, but may skip stale frames.

bool Swapchain::PresentMode(bool no_tearing, bool powersave) {
    return PresentMode(VkPresentModeKHR ((no_tearing ? 1 : 0) ^ (powersave ? 3 : 0)));  // if not found, use FIFO
}

bool Swapchain::PresentMode(VkPresentModeKHR pref_mode) {
    VkPresentModeKHR& mode = info.presentMode;
    auto modes = GetPresentModes(gpu, surface);
    mode = VK_PRESENT_MODE_FIFO_KHR;                           // default to FIFO mode
    for (auto m : modes) if(m == pref_mode) mode = pref_mode;  // if prefered mode is available, select it.
    if (mode != pref_mode) LOGW("Requested present-mode is not supported. Reverting to FIFO mode.\n");
    if (!!swapchain) Apply();
    return (mode == pref_mode);
}

const char* PresentModeName(VkPresentModeKHR mode) {
    switch(mode) {                                                            //  no_tearing  |powersave
        case 0 :          return "VK_PRESENT_MODE_IMMEDIATE_KHR";             //  FALSE       |FALSE
        case 1 :          return "VK_PRESENT_MODE_MAILBOX_KHR";               //  TRUE        |FALSE
        case 2 :          return "VK_PRESENT_MODE_FIFO_KHR";                  //  TRUE        |TRUE
        case 3 :          return "VK_PRESENT_MODE_FIFO_RELAXED_KHR";          //  FALSE       |TRUE
        case 1000111000 : return "VK_PRESENT_MODE_SHARED_DEMAND_REFRESH_KHR";
        case 1000111001 : return "VK_PRESENT_MODE_SHARED_CONTINUOUS_REFRESH_KHR";
        default :         return "UNKNOWN";
    };
}
//-----------------------------------------------------------------------

// Framebuffer formats
const char* FormatStr(VkFormat fmt) {
#define STR(f) case f: return #f
    switch (fmt) {
        STR(VK_FORMAT_UNDEFINED);            //  0
        //Color
        STR(VK_FORMAT_R5G6B5_UNORM_PACK16);  //  4
        STR(VK_FORMAT_R8G8B8A8_UNORM);       // 37
        STR(VK_FORMAT_R8G8B8A8_SRGB);        // 43
        STR(VK_FORMAT_B8G8R8A8_UNORM);       // 44
        STR(VK_FORMAT_B8G8R8A8_SRGB);        // 50
        //Depth
        STR(VK_FORMAT_D32_SFLOAT);           //126
        STR(VK_FORMAT_D32_SFLOAT_S8_UINT);   //130
        STR(VK_FORMAT_D24_UNORM_S8_UINT);    //129
        STR(VK_FORMAT_D16_UNORM_S8_UINT);    //128
        STR(VK_FORMAT_D16_UNORM);            //124
        STR(VK_FORMAT_X8_D24_UNORM_PACK32);  //125
        default: return "";
    }
#undef STR
}

void Swapchain::Print() {
    printf("Swapchain:\n");

    printf("\tFormat  = %3d : %s\n", info.imageFormat,    FormatStr(info.imageFormat));
    printf("\tDepth   = %3d : %s\n", depth_buffer.format, FormatStr(depth_buffer.format));

    VkExtent2D& extent = info.imageExtent;
    printf("\tExtent  = %d x %d\n", extent.width, extent.height);
    printf("\tBuffers = %d\n", (int)buffers.size());

    auto modes = GetPresentModes(gpu, surface);
    printf("\tPresentMode:\n");
    VkPresentModeKHR& mode = info.presentMode;
    for (auto m : modes) print((m == mode) ? eRESET : eFAINT, "\t\t%s %s\n", (m == mode) ? cTICK : " ",PresentModeName(m));
    printf("\n");
}

void Swapchain::Apply() {
    ASSERT(renderpass, "CSwapchain: No Renderpass selected.");
    info.oldSwapchain = swapchain;
    VKERRCHECK(vkCreateSwapchainKHR(device, &info, nullptr, &swapchain));

    //-- Delete old swapchain --
    if (info.oldSwapchain) {
        Clear();
        vkDestroySwapchainKHR(device, info.oldSwapchain, 0);
    }
    //--------------------------

    //-- Allocate array of images for swapchain--
    std::vector<VkImage> images;
    uint32_t count = 0;
    VKERRCHECK(vkGetSwapchainImagesKHR(device, swapchain, &count, nullptr));
    images.resize(count);
    VKERRCHECK(vkGetSwapchainImagesKHR(device, swapchain, &count, images.data()));
    buffers.resize(count);
    //-------------------------------------------

    ResizeAttachments();

    repeat(count) { // buffers
        auto& buf = buffers[i];
        buf.image = images[i];

        //---ImageView---
        VkImageViewCreateInfo ivCreateInfo = {VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO};
        ivCreateInfo.pNext    = NULL;
        ivCreateInfo.flags    = 0;
        ivCreateInfo.image    = buf.image;  //images[i];
        ivCreateInfo.format   = info.imageFormat;
        ivCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        ivCreateInfo.components = {};
        ivCreateInfo.subresourceRange.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
        ivCreateInfo.subresourceRange.baseMipLevel   = 0;
        ivCreateInfo.subresourceRange.levelCount     = 1;
        ivCreateInfo.subresourceRange.baseArrayLayer = 0;
        ivCreateInfo.subresourceRange.layerCount     = 1;
        VKERRCHECK(vkCreateImageView(device, &ivCreateInfo, nullptr, &buf.view));
        //---------------

        //--Attachment View list--
        uint32_t count = (uint32_t)renderpass->attachments.size();
        std::vector<VkImageView> views(count);
        uint32_t ctr = 0;
        repeat(count) {
            switch(renderpass->attachments[i].finalLayout) {
              case VK_IMAGE_LAYOUT_PRESENT_SRC_KHR                  : views[i] = buf.view;                 break;
              case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL : views[i] = depth_buffer.view;        break;
              case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL         : views[i] = att_images[ctr++].view;   break;
              default : LOGE("Swapchain: Attachment type not supported\n");
            }
        }
        //------------------------

        //--Framebuffer--
        VkFramebufferCreateInfo fbCreateInfo = {VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO};
        fbCreateInfo.renderPass      = *renderpass;
        fbCreateInfo.attachmentCount = (uint32_t)views.size();  // 1/2
        fbCreateInfo.pAttachments    =           views.data();  // views for color and depth buffer
        fbCreateInfo.width  = info.imageExtent.width;
        fbCreateInfo.height = info.imageExtent.height;
        fbCreateInfo.layers = 1;
        VKERRCHECK(vkCreateFramebuffer(device, &fbCreateInfo, nullptr, &buf.framebuffer));
        //---------------
        //--CommandBuffer--
        if(!buf.command_buffer) {
            VkCommandBufferAllocateInfo allocInfo = {VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO};
            allocInfo.commandPool = command_pool;
            allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
            allocInfo.commandBufferCount = 1;
            VKERRCHECK(vkAllocateCommandBuffers(device, &allocInfo, &buf.command_buffer));
        }
        //-----------------
        //---Create Fence---
        VkFenceCreateInfo createInfo = {VK_STRUCTURE_TYPE_FENCE_CREATE_INFO};
        createInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
        vkCreateFence(device, &createInfo, nullptr, &buf.fence);
        //------------------
        // ---Create Semaphores---
        VkSemaphoreCreateInfo semaphoreInfo = {VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO};
        VKERRCHECK(vkCreateSemaphore(device, &semaphoreInfo, nullptr, &buf.acquire_semaphore));
        VKERRCHECK(vkCreateSemaphore(device, &semaphoreInfo, nullptr, &buf.submit_semaphore));
        // -----------------------

        //printf("---Extent = %d x %d\n", info.imageExtent.width, info.imageExtent.height);
    }
    if (!info.oldSwapchain) LOGI("Swapchain created\n");
}
//---------------------------------------------------------------------------------

FrameBuffer& Swapchain::AcquireNext() {
    ASSERT(!is_acquired, "CSwapchain: Previous swapchain buffer has not yet been presented.\n");
    SetExtent();  // TODO: Remove?

    // Acquire next image
    uint32_t prev = acquired_index;
    VkSemaphore acquire_semaphore = CurrBuffer().acquire_semaphore;
    VkResult result = vkAcquireNextImageKHR(device, swapchain, UINT64_MAX, acquire_semaphore, VK_NULL_HANDLE, &acquired_index);
    //ShowVkResult(result);
    // window resized (for Nvidia GPU)
    while(result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR) {
        SetExtent();
        acquire_semaphore = CurrBuffer().acquire_semaphore;
        result = vkAcquireNextImageKHR(device, swapchain, UINT64_MAX, acquire_semaphore, VK_NULL_HANDLE, &acquired_index);
    }

    FrameBuffer& buf = buffers[acquired_index];
    buf.extent = info.imageExtent;
    vkWaitForFences(device, 1, &buf.fence, VK_TRUE, UINT64_MAX);
    rendered_index = prev;
    is_acquired = true;
    return buf;
}

void Swapchain::Submit() {  // and present
    ASSERT(!!is_acquired, "CSwapchain: A buffer must be acquired before presenting.\n");
    VkSemaphore acquire_semaphore = PrevBuffer().acquire_semaphore;
    VkSemaphore submit_semaphore  = CurrBuffer().submit_semaphore;

    // --- Submit ---
    FrameBuffer& buf = CurrBuffer();
    VkSubmitInfo submitInfo = {VK_STRUCTURE_TYPE_SUBMIT_INFO};
    VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    submitInfo.waitSemaphoreCount   = 1;
    submitInfo.pWaitSemaphores      = &acquire_semaphore;  // wait for acquire to complete, before submitting
    submitInfo.pWaitDstStageMask    = waitStages;
    submitInfo.commandBufferCount   = 1;
    submitInfo.pCommandBuffers      = &buf.command_buffer;
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores    = &submit_semaphore;   // signal when submit has completed
    vkResetFences(device, 1, &buf.fence);
    VKERRCHECK(vkQueueSubmit(graphics_queue, 1, &submitInfo, buf.fence));  // Submit new render commands

    // --- Present ---
    VkPresentInfoKHR presentInfo = {};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores    = &submit_semaphore;  // wait for submit to complete, before presenting
    presentInfo.swapchainCount     = 1;
    presentInfo.pSwapchains        = &swapchain;
    presentInfo.pImageIndices      = &acquired_index;
    //VKERRCHECK(vkQueuePresentKHR(present_queue, &presentInfo));
    VkResult result = vkQueuePresentKHR(present_queue, &presentInfo);
    //ShowVkResult(result);
/*
    {   // Window resize (for NVidia GPU.  (Warning: doesn't work on Intel GPU))
        if(result == VK_ERROR_OUT_OF_DATE_KHR) SetExtent();
        else ShowVkResult(result);
    }
    {   // Window resize (for Intel GPU)
        VKERRCHECK(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(gpu, surface, &surface_caps));
        VkExtent2D& curr = surface_caps.currentExtent;
        VkExtent2D& ext = info.imageExtent;
        //printf("swapchain: w=%d h=%d curr_w=%d curr_h=%d\n", ext.width, ext.height, curr.width, curr.height);
        if(curr.width != ext.width || curr.height != ext.height) SetExtent();
    }
*/
    is_acquired = false;
}

CImage& Swapchain::ReadImage() {
    format = info.imageFormat;
    return FBO::ReadImage();
}
