#ifndef CPIPELINE_H
#define CPIPELINE_H

#include "CRenderpass.h"
#include "CShader.h"

class CPipeline {
    VkDevice     device          =0;
    VkRenderPass renderpass      =0;
    uint32_t     subpass         =0;
    VkPipeline   graphicsPipeline=0;
    VkViewport   viewport = {};
    VkRect2D     scissor = {};
    std::vector<CvkImage*> input_attachments;
  public:
    CShader shader;

    CPipeline();
    CPipeline(CRenderpass& renderpass, uint32_t subpass = 0);
    void Init(CRenderpass& renderpass, uint32_t subpass = 0);
    void Init(VkDevice device, VkRenderPass renderpass, uint32_t subpass, VkSampleCountFlagBits samples, std::vector<CvkImage*> input_attachments);
    ~CPipeline();

    VkPipelineInputAssemblyStateCreateInfo inputAssembly        = {VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO};
    VkPipelineViewportStateCreateInfo      viewportState        = {VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO};
    VkPipelineRasterizationStateCreateInfo rasterizer           = {VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO};
    VkPipelineMultisampleStateCreateInfo   multisample          = {VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO};
    VkPipelineDepthStencilStateCreateInfo  depthStencilState    = {VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO};
    //VkPipelineColorBlendAttachmentState    colorBlendAttachment = {};
    VkPipelineColorBlendAttachmentState    colorBlendAttachment[4] = {};
    VkPipelineColorBlendStateCreateInfo    colorBlending        = {VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO};

    void SetDefaults();
    void EnableAlphaBlend(uint32_t att_index = 0);
    VkPipeline CreateGraphicsPipeline();
    void Destroy();
    operator VkPipeline() const { return graphicsPipeline; }
    void Bind(VkCommandBuffer cmd, VkDescriptorSets& ds);
};



#endif

