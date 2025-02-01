// * Copyright (C) 2019 by Rene Lindsay
//
//      -- Renderpasss structure --
//
//                    +-------------------------+
//                    | VkAttachmentDescription |
//                    +-----+--------------+----+
//                          | 1            V m
//                          ^ m            |
//           +--------------+--------+     |
//           | VkAttachmentReference |     |
//           +--------------+--------+     |
//                          V m            |
//                          | 1            |
//           +--------------+-------+      |
//           | VkSubpassDescription |      |
//           +---------+----+-------+      |
//                     V 2  V m            |
//                     | 1  |              |
//  +------------------+--+ |              |
//  | VkSubpassDependency | |              |
//  +------------------+--+ |              |
//                     V m  |              |
//                     |    |              |
//                     | 1  | 1            | 1
//                   +------+--------------+--+
//                   | VkRenderPassCreateInfo |
//                   +------------------------+
//
// How to use:
//   1: Find which device can present to given surface, and what color/depth formats it supports.
//   2: Create an instance of renderpass. eg: CRenderpass renderpass(device);
//   3: Add Color and Depth-stencil attachments.
//   4: Add subpasses, with an array of attachment indexes, used by that renderpass.
//
// eg:
//    CRenderpass renderpass(device 2);                                                 // Create a renderpass with 2 subpasses.
//    uint32_t present = renderpass.NewPresentAttachment(VK_FORMAT_B8G8R8A8_UNORM);     // Create presentable color attachment  (index 0)
//    uint32_t color   = renderpass.NewColorAttachment  (VK_FORMAT_B8G8R8A8_UNORM);     // Create offscreen color attachment    (index 1)
//    uint32_t depth   = renderpass.NewDepthAttachment  (VK_FORMAT_D24_UNORM_S8_UINT);  // Create depth-stencil attachment      (index 2)
//    renderpass.subpasses[0].AddColorAttachment(color);                                // Write to color attachment in subpass_0
//    renderpass.subpasses[0].AddDepthAttachment(depth);                                // Write to depth attachment in subpass_0
//    renderpass.subpasses[1].AddInputAttachment(color);                                // Read from color attachment  in subpass_1
//    renderpass.subpasses[1].AddColorAttachment(present);                              // Write to present attachment in subpass_1
//    renderpass.AddSubpassDependency(0,1);                                             // subpass_1 depends on subpass_0
//    renderpass.Create();                                                              // Create the VkRenderPass instance. (optional)
//
//    NOTE: Get supported formats from CPhysicalDevice, rather than hard-coding them.
//    WARNING: Renderpass can't be modified after passing it to Swapchain or CPipeline
//
// TODO:
//   Subpass.pResolveAttachments

#ifndef CRENDERPASS_H
#define CRENDERPASS_H

#include "vkWindow.h"
#include "vkImages.h"
#include <vector>

/*
struct Attachment {
    uint32_t                index;
    VkAttachmentDescription description;
    VkClearValue            clearval;
};
*/

class CRenderpass {
    class CSubpass {
        friend class CRenderpass;
        friend class FBO;
        CRenderpass* renderpass = 0;
        std::vector<VkAttachmentReference> input_refs;     // read
        std::vector<VkAttachmentReference> color_refs;     // write
        std::vector<VkAttachmentReference> resolve_refs;   // msaa
        std::vector<uint32_t>              preserve_refs;  // preserve
        VkAttachmentReference              depth_ref{};
        operator VkSubpassDescription();
        void AddResolveAttachment (uint32_t index);  // automatically added if MSAA samples > 1
    public:
        CSubpass(){};
        CSubpass(CRenderpass* renderpass) : renderpass(renderpass){}
        void AddInputAttachment   (uint32_t index);  // read-only attachments
        void AddColorAttachment   (uint32_t index);  // write attachments
        void AddDepthAttachment   (uint32_t index);  // only one depth buffer allowed
        void AddPreserveAttachment(uint32_t index);  // not used in this subpass
        VkSampleCountFlagBits samples = VK_SAMPLE_COUNT_1_BIT;
        std::vector<CvkImage*> input_att;
    };
    
    friend class CPipeline;
    VkDevice     device     = 0;
    VkRenderPass renderpass = 0;

  public:
    std::vector<VkClearValue>            clearValues;
    std::vector<VkAttachmentDescription> attachments;
    std::vector<CSubpass>                subpasses;
    std::vector<VkSubpassDependency>     dependencies;

    CRenderpass(VkDevice device, uint32_t subpass_count = 1);
    CRenderpass();
    ~CRenderpass();
    void Init(VkDevice device, uint32_t subpass_count = 1);

    uint32_t AddAttachment       (VkAttachmentDescription description, VkClearValue clearVal = {});
    uint32_t NewPresentAttachment(VkFormat format, VkSampleCountFlagBits samples = VK_SAMPLE_COUNT_1_BIT, VkClearColorValue clearVal = {});
    uint32_t NewColorAttachment  (VkFormat format, VkSampleCountFlagBits samples = VK_SAMPLE_COUNT_1_BIT, VkClearColorValue clearVal = {});
    uint32_t NewDepthAttachment  (VkFormat format, VkSampleCountFlagBits samples = VK_SAMPLE_COUNT_1_BIT, VkClearDepthStencilValue clearVal = {1.f, 0});
    uint32_t ResolveAttachment   (uint32_t index);

    void AddSubpassDependency(uint32_t srcSubpass, uint32_t dstSubpass);

    VkAttachmentDescription* GetDepthAttachment();    // returns the depth-attachment's description struct
    VkAttachmentDescription* GetPresentAttachment();  // returns the presentable color-attachment's description struct
    VkFormat GetDepthFormat();                        // used by Swapchain
    VkFormat GetPresentFormat();                      // used by Swapchain

    uint32_t ColorAttachmentCount();  // returns number of color attachments
    std::vector<VkAttachmentDescription*> GetColorAttachments();  // returns the list of color-attachment's descriptions

    void Create();
    void Destroy();
    void Print();
    operator VkRenderPass () {
        if(!renderpass) Create();
        return renderpass;
    }
};

#endif
