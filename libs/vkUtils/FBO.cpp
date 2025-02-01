#include "FBO.h"
#include "VkFormats.h"

FBO::FBO(CRenderpass& renderpass, const CQueue* graphics_queue) {
    Init(renderpass, graphics_queue);
}

void FBO::Init(CRenderpass& renderpass, const CQueue* graphics_queue) {
    ASSERT(!this->renderpass, "Renderpass already selected.");
    this->renderpass = &renderpass;
    Init(graphics_queue);
}

FBO::~FBO() {
    if(device) Clear();
    if(command_pool) vkDestroyCommandPool(device, command_pool, nullptr);  command_pool=0;
    LOGI("FBO destroyed\n");
}

void FBO::Clear() {
    vkDeviceWaitIdle(device);
    for(auto& buf : buffers) {
        if(buf.fence) {
            vkWaitForFences(device, 1, &buf.fence, VK_TRUE, UINT64_MAX);
            VKERRCHECK(vkGetFenceStatus(device, buf.fence));
            vkDestroyFence      (device, buf.fence,       nullptr);  buf.fence      =nullptr;
            vkDestroyFramebuffer(device, buf.framebuffer, nullptr);  buf.framebuffer=nullptr;
            //vkDestroyImageView  (device, buf.view,        nullptr);  buf.view       =nullptr;
        }
    }
}

void FBO::Init(const CQueue* graphics_queue) {
    this->gpu     = graphics_queue->gpu;
    this->device  = graphics_queue->device;
    this->graphics_queue = *graphics_queue;
    is_acquired   = false;

    //VkInstance instance = default_allocator->instance;
    //export_allocator.Init(instance, *graphics_queue);

    SetExtent(256,256);
    command_pool = graphics_queue->CreateCommandPool();
    auto* att = renderpass->GetDepthAttachment();
    if(att) depth_buffer.SetSize(extent, att->format, att->samples);
    SetFramebufferCount(2);
}

void FBO::SetExtent(uint32_t width, uint32_t height) {
    if((extent.width == width)&&(extent.height == height)) return;
    extent = {width, height};
    Apply();
}

bool FBO::SetFramebufferCount(uint32_t count) {  // set number of framebuffers. (2 or 3)
    Clear();
    buffers.resize(count);
    images.resize(count);
    Apply();
    return true;
}

void FBO::ResizeAttachments() {
    if(depth_buffer.image) depth_buffer.Resize(extent);  //resize depth buffer

    //---Additional attachments, besides depth and present ---
    uint32_t att_cnt = renderpass->ColorAttachmentCount();
    att_images.resize(att_cnt);
    //--------------------------------------------------------

    std::map<uint, uint> inmap;  // map input-attachment-index to att_image-index

    //---resize attachments to fit framebuffer size---
    uint32_t inx = 0;
    uint32_t ctr = 0;
    for(auto& att : renderpass->attachments) {
        if(ctr<att_images.size()) {
            if (att.finalLayout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL) {
                if(att.samples==1) inmap[inx]=ctr;  // input attachments
                auto& attImg = att_images[ctr++];
                VkImageUsageFlags usage =   VK_IMAGE_USAGE_TRANSFER_SRC_BIT |
                                            VK_IMAGE_USAGE_TRANSFER_DST_BIT |
                                            //VK_IMAGE_USAGE_SAMPLED_BIT |
                                            VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT |
                                            VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT;
                if(att.samples==1) usage |= VK_IMAGE_USAGE_STORAGE_BIT;
                attImg.SetSize(extent, att.format, att.samples, usage);
                attImg.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
            }
        }
        ++inx;
    }
    //------------------------------------------------

    // --- Populate subpass[].input_att ---
    for(auto& sub : renderpass->subpasses) {
        sub.input_att.clear();
        for(auto& inref : sub.input_refs)
            sub.input_att.push_back(&att_images[inmap[inref.attachment]]);
    }
    //-------------------------------------
}

void FBO::Apply() {
    ASSERT(renderpass, "FBO: No Renderpass selected.\n");
    Clear();

    {  // --- Offscreen render targets ---
        VkImageUsageFlags usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT |
                                  VK_IMAGE_USAGE_TRANSFER_DST_BIT |
                                  //VK_IMAGE_USAGE_SAMPLED_BIT |
                                  //VK_IMAGE_USAGE_STORAGE_BIT |
                                  VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT |
                                  VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT |
                                  0;

        format = renderpass->GetPresentFormat();
        if(!format) format = renderpass->GetColorAttachments()[0]->format;  // TODO: MRT

        for(auto& image : images) {  // for each framebuffer
            //image.allocator = &export_allocator;
            image.SetSize(extent, format, VK_SAMPLE_COUNT_1_BIT, usage);
        }
    }  // --------------------------------

    ResizeAttachments();

    uint32_t count = (uint32_t)buffers.size();

    repeat(count) { // buffers
        auto& buf = buffers[i];
        buf.image = images[i];
        buf.view  = images[i].view;

        //--Attachment View list--
        uint32_t count = (uint32_t)renderpass->attachments.size();
        std::vector<VkImageView> views(count);
        uint32_t ctr = 0;
        repeat(count) {
            switch(renderpass->attachments[i].finalLayout) {
              case VK_IMAGE_LAYOUT_PRESENT_SRC_KHR                  : views[i] = buf.view;                break;
              case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL : views[i] = depth_buffer.view;       break;
              case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL         : views[i] = att_images[ctr++].view;  break;
              default : LOGE("FBO: Attachment type not supported\n");
            }
        }
        //------------------------

        //--Framebuffer--
        VkFramebufferCreateInfo fbCreateInfo = {VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO};
        fbCreateInfo.renderPass      = *renderpass;
        fbCreateInfo.attachmentCount = (uint32_t)views.size();  // 1/2
        fbCreateInfo.pAttachments    =           views.data();  // views for color and depth buffer
        fbCreateInfo.width  = extent.width;
        fbCreateInfo.height = extent.height;
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
        //---Fence---
        VkFenceCreateInfo createInfo = {VK_STRUCTURE_TYPE_FENCE_CREATE_INFO};
        createInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
        vkCreateFence(device, &createInfo, nullptr, &buf.fence);
        //-----------
    }
}
//------------------------------------------------

FrameBuffer& FBO::AcquireNext() {
    ASSERT(!is_acquired, "FBO: Previous framebuffer has not yet been presented.\n");
    uint32_t prev = acquired_index;

    uint32_t count = (uint32_t)buffers.size();
    acquired_index = (acquired_index + 1) % count;

    FrameBuffer& buf = buffers[acquired_index];
    buf.extent = extent;
    vkWaitForFences(device, 1, &buf.fence, VK_TRUE, UINT64_MAX);
    rendered_index = prev;
    is_acquired = true;
    return buf;
}

void FBO::Submit() {
    ASSERT(!!is_acquired, "FBO: A buffer must be acquired before presenting.\n");
    // --- Submit ---
    FrameBuffer& buffer = buffers[acquired_index];
    VkSubmitInfo submitInfo = {VK_STRUCTURE_TYPE_SUBMIT_INFO};
    VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    submitInfo.pWaitDstStageMask    = waitStages;
    submitInfo.commandBufferCount   = 1;
    submitInfo.pCommandBuffers      = &buffer.command_buffer;

    vkResetFences(device, 1, &buffer.fence);
    VKERRCHECK(vkQueueSubmit(graphics_queue, 1, &submitInfo, buffer.fence));
    //if(wait) { VKERRCHECK(vkWaitForFences(device, 1, &buffer.fence, VK_TRUE, UINT64_MAX)) }
    // --- Present ---
    //...
    //----------------
    VkExtent2D& ext  = extent;
    VkExtent2D& curr = buffer.extent;
    if(curr.width != ext.width || curr.height != ext.height) SetExtent(ext.width, ext.height);

    is_acquired = false;
}

void FBO::Wait() {
    FrameBuffer& buffer = buffers[acquired_index];
    VKERRCHECK(vkWaitForFences(device, 1, &buffer.fence, VK_TRUE, UINT64_MAX));
}
//---------------------------------------------------------------------------------

VkCommandBuffer FBO::BeginFrame() {
    AcquireNext();
    auto cmd_buf = BeginCmd();
    BeginRenderpass();
    return cmd_buf;
}

void FBO::EndFrame() {
    EndRenderpass();
    EndCmd();
    Submit();
    Wait();
}

//---------------------------------------------------------------------------------

VkCommandBuffer FBO::BeginCmd() {
    auto& swap_buf = buffers[acquired_index];
    auto& cmd_buf = swap_buf.command_buffer;
    VkCommandBufferBeginInfo beginInfo = {VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO};
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
    VKERRCHECK(vkBeginCommandBuffer(cmd_buf, &beginInfo));
    return cmd_buf;
}

void FBO::EndCmd() {
    auto& cmd_buf = buffers[acquired_index].command_buffer;
    VKERRCHECK(vkEndCommandBuffer(cmd_buf));
}

void FBO::BeginRenderpass() {
    auto& swap_buf = buffers[acquired_index];
    auto& cmd_buf = swap_buf.command_buffer;

    VkRenderPassBeginInfo renderPassInfo = {VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO};
    renderPassInfo.renderPass = *renderpass;
    renderPassInfo.framebuffer = swap_buf.framebuffer;
    renderPassInfo.renderArea.offset = {0, 0};
    renderPassInfo.renderArea.extent = swap_buf.extent;
    renderPassInfo.clearValueCount = (uint32_t)renderpass->clearValues.size();
    renderPassInfo.pClearValues    =           renderpass->clearValues.data();
    vkCmdBeginRenderPass(cmd_buf, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
}

void FBO::EndRenderpass() {
    auto& swap_buf = buffers[acquired_index];
    auto& cmd_buf = swap_buf.command_buffer;
    vkCmdEndRenderPass(cmd_buf);
}

//---------------------------------------------------------------------------------

CImage& FBO::ReadImage() {
    //Wait();
    FrameBuffer fb = CurrBuffer();
    uint32_t w = fb.extent.width;
    uint32_t h = fb.extent.height;
    VkExtent3D extent = {w, h, 1};
    //VkFormat format   = info.imageFormat;
    format_info fmt = FormatInfo(format);
    static CImage img;  // Reuse buffer
    img.SetSize(w,h);
    ASSERT(fmt.size==4, "ReadImage: FB format not supported by CImage.");
    if(fmt.type==UNORM) img.colorspace = csUNORM;
    if(fmt.type==SRGB)  img.colorspace = csSRGB;
    VkImageLayout layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    default_allocator->ReadImage(fb.image, layout, extent, format, img.Buffer());
    //img.Clear({255,255,255,255}, false,false,false,true);  // set alpha to 255
    //if(fmt.isSwizzled()) img.BGRAtoRGBA();
    return img;
}
