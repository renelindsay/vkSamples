#undef  _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS

#include "CPipeline.h"
#include "vkWindow.h"

CPipeline::CPipeline() :device(), renderpass(), graphicsPipeline() {}

CPipeline::CPipeline(CRenderpass& renderpass, uint32_t subpass) {
    Init(renderpass, subpass);
}

void CPipeline::Init(CRenderpass& renderpass, uint32_t subpass) {
    auto& sub = renderpass.subpasses[subpass];
    Init(renderpass.device, renderpass, subpass, sub.samples, sub.input_att);
}

void CPipeline::Init(VkDevice device, VkRenderPass renderpass, uint32_t subpass, VkSampleCountFlagBits samples, std::vector<CvkImage*> input_attachments) {
    shader.Init(device);
    this->device = device;
    this->renderpass = renderpass;
    this->subpass = subpass;
    this->input_attachments = input_attachments;
    SetDefaults();
    multisample.rasterizationSamples = samples;
    if(samples==1) multisample.sampleShadingEnable = false;
}

CPipeline::~CPipeline() {
    Destroy();
}

void CPipeline::Destroy() {
    if (device) vkDeviceWaitIdle(device);
    if (graphicsPipeline) vkDestroyPipeline(device, graphicsPipeline, nullptr);
}

void CPipeline::SetDefaults() {
    //VkPipelineInputAssemblyStateCreateInfo inputAssembly = {VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO};
    inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    inputAssembly.primitiveRestartEnable = VK_FALSE;

    VkExtent2D extent = {640,480};
    float w = extent.width;
    float h = extent.height;
    viewport = {0, 0, w, h, 0, 1};  // vulkan style (y=down)
    //viewport = {0, h, w,-h, 0, 1};    // opengl style (y=up)

    scissor.offset = {0, 0};
    scissor.extent = extent;

    //VkPipelineViewportStateCreateInfo viewportState = {VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO};
    viewportState.viewportCount = 1;
    viewportState.pViewports = &viewport;
    viewportState.scissorCount = 1;
    viewportState.pScissors = &scissor;

    //VkPipelineRasterizationStateCreateInfo rasterizer = {VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO};
    rasterizer.depthClampEnable = VK_FALSE;
    rasterizer.rasterizerDiscardEnable = VK_FALSE;
    rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
    rasterizer.lineWidth = 1.0f;
    rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
    //rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
    rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;  // glTF front faces are CCW
    rasterizer.depthBiasEnable = VK_FALSE;

    //VkPipelineMultisampleStateCreateInfo multisample = {VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO};
    multisample.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;  // must match renderpass.samples
    multisample.sampleShadingEnable  = VK_TRUE;                // False=MSAA True=SSAA
    multisample.minSampleShading     = 1.f;                    // Run shader for ALL samples

    //VkPipelineDepthStencilStateCreateInfo depthStencilState = {VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO};
    depthStencilState.depthTestEnable       = VK_TRUE;
    depthStencilState.depthWriteEnable      = VK_TRUE;
    depthStencilState.depthCompareOp        = VK_COMPARE_OP_LESS_OR_EQUAL;
    depthStencilState.depthBoundsTestEnable = VK_FALSE;
    depthStencilState.stencilTestEnable     = VK_FALSE;
    //depthStencilState.front.failOp          = VK_STENCIL_OP_KEEP;
    //depthStencilState.front.passOp          = VK_STENCIL_OP_KEEP;
    //depthStencilState.front.compareOp       = VK_COMPARE_OP_ALWAYS;
    //depthStencilState.back.failOp           = VK_STENCIL_OP_KEEP;
    //depthStencilState.back.passOp           = VK_STENCIL_OP_KEEP;
    //depthStencilState.back.compareOp        = VK_COMPARE_OP_ALWAYS;
    //depthStencilState.minDepthBounds        = 0;
    //depthStencilState.maxDepthBounds        = MAXFLOAT;

    //colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    //colorBlendAttachment.blendEnable = VK_FALSE;

    repeat(4) {
        colorBlendAttachment[i].colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
        colorBlendAttachment[i].blendEnable = VK_FALSE;
    }

    //VkPipelineColorBlendStateCreateInfo colorBlending = {VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO};
    colorBlending.logicOpEnable = VK_FALSE;
    colorBlending.logicOp = VK_LOGIC_OP_COPY;
    colorBlending.attachmentCount = 1;
    colorBlending.pAttachments = colorBlendAttachment;
    colorBlending.blendConstants[0] = 0.0f;
    colorBlending.blendConstants[1] = 0.0f;
    colorBlending.blendConstants[2] = 0.0f;
    colorBlending.blendConstants[3] = 0.0f;
}

// Enable alpha-blending
//   finalColor.rgb = newAlpha * newColor + (1 - newAlpha) * oldColor;
//   finalColor.a = newAlpha.a;
void CPipeline::EnableAlphaBlend(uint32_t fb_inx) {
    VkPipelineColorBlendAttachmentState& cb = colorBlendAttachment[fb_inx];
    cb.blendEnable         = VK_TRUE;
    cb.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
    cb.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    cb.colorBlendOp        = VK_BLEND_OP_ADD;
    cb.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
    cb.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
    cb.alphaBlendOp        = VK_BLEND_OP_ADD;
}

VkPipeline CPipeline::CreateGraphicsPipeline() {
    ASSERT(device, "Pipeline not initialized.");
    Destroy();

    VkPipelineDynamicStateCreateInfo dynamicState = {VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO};
    VkDynamicState dynamicStates[] = {VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR};
    dynamicState.pDynamicStates    = dynamicStates;
    dynamicState.dynamicStateCount = sizeof(dynamicStates)/sizeof(VkDynamicState);  //2

    VkGraphicsPipelineCreateInfo pipelineInfo = {VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO};
    pipelineInfo.stageCount = (uint32_t)shader.shaderStages.size();
    pipelineInfo.pStages              = shader.shaderStages.data();
    pipelineInfo.pVertexInputState   = &shader.vertexInputs;
    pipelineInfo.pInputAssemblyState = &inputAssembly;
    //pipelineInfo.pTessellationState = 0;
    pipelineInfo.pViewportState      = &viewportState;
    pipelineInfo.pRasterizationState = &rasterizer;
    pipelineInfo.pMultisampleState   = &multisample;
    pipelineInfo.pDepthStencilState  = &depthStencilState;
    pipelineInfo.pColorBlendState    = &colorBlending;
    pipelineInfo.pDynamicState       = &dynamicState;
    pipelineInfo.layout              = shader.GetPipelineLayout();
    pipelineInfo.renderPass          = renderpass;
    pipelineInfo.subpass             = subpass;
    pipelineInfo.basePipelineHandle  = VK_NULL_HANDLE;
    //pipelineInfo.basePipelineIndex = 0;

    VKERRCHECK(vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &graphicsPipeline));
    return graphicsPipeline;
}

void CPipeline::Bind(VkCommandBuffer cmd, VkDescriptorSets& ds) {
    if(subpass>0) {
        shader.BindInputAttachments(input_attachments);
        shader.UpdateDescriptorSets(ds);
    }
    VkPipelineLayout pipelineLayout = shader.GetPipelineLayout();
    vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, *this);
    vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, ds, 0, nullptr);
}











/*
VkPipeline CPipeline::CreateGraphicsPipeline() {
    //VkPipelineInputAssemblyStateCreateInfo inputAssembly = {VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO};
    inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    inputAssembly.primitiveRestartEnable = VK_FALSE;

    VkExtent2D extent = {64,64};
    VkViewport viewport = {};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = (float) extent.width;
    viewport.height = (float) extent.height;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    VkRect2D scissor = {};
    scissor.offset = {0, 0};
    scissor.extent = extent;

    //VkPipelineViewportStateCreateInfo viewportState = {VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO};
    viewportState.viewportCount = 1;
    viewportState.pViewports = &viewport;
    viewportState.scissorCount = 1;
    viewportState.pScissors = &scissor;

    //VkPipelineRasterizationStateCreateInfo rasterizer = {VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO};
    rasterizer.depthClampEnable = VK_FALSE;
    rasterizer.rasterizerDiscardEnable = VK_FALSE;
    rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
    rasterizer.lineWidth = 1.0f;
    rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
    //rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
    rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;  // glTF front faces are CCW
    rasterizer.depthBiasEnable = VK_FALSE;

    //VkPipelineMultisampleStateCreateInfo multisampling = {VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO};
    multisampling.sampleShadingEnable = VK_FALSE;
    multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

    //VkPipelineDepthStencilStateCreateInfo depthStencilState = {VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO};
    depthStencilState.depthTestEnable       = VK_TRUE;
    depthStencilState.depthWriteEnable      = VK_TRUE;
    depthStencilState.depthCompareOp        = VK_COMPARE_OP_LESS_OR_EQUAL;
    depthStencilState.depthBoundsTestEnable = VK_FALSE;
    depthStencilState.stencilTestEnable     = VK_FALSE;
    //depthStencilState.front.failOp          = VK_STENCIL_OP_KEEP;
    //depthStencilState.front.passOp          = VK_STENCIL_OP_KEEP;
    //depthStencilState.front.compareOp       = VK_COMPARE_OP_ALWAYS;
    //depthStencilState.back.failOp           = VK_STENCIL_OP_KEEP;
    //depthStencilState.back.passOp           = VK_STENCIL_OP_KEEP;
    //depthStencilState.back.compareOp        = VK_COMPARE_OP_ALWAYS;
    //depthStencilState.minDepthBounds        = 0;
    //depthStencilState.maxDepthBounds        = MAXFLOAT;

    //VkPipelineColorBlendAttachmentState colorBlendAttachment = {};
    colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    colorBlendAttachment.blendEnable = VK_FALSE;

    //VkPipelineColorBlendStateCreateInfo colorBlending = {VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO};
    colorBlending.logicOpEnable = VK_FALSE;
    colorBlending.logicOp = VK_LOGIC_OP_COPY;
    colorBlending.attachmentCount = 1;
    colorBlending.pAttachments = &colorBlendAttachment;
    colorBlending.blendConstants[0] = 0.0f;
    colorBlending.blendConstants[1] = 0.0f;
    colorBlending.blendConstants[2] = 0.0f;
    colorBlending.blendConstants[3] = 0.0f;

    VkDynamicState dynamicStates[VK_DYNAMIC_STATE_RANGE_SIZE] = {VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR};
    VkPipelineDynamicStateCreateInfo dynamicState = {VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO};
    dynamicState.pDynamicStates = dynamicStates;
    dynamicState.dynamicStateCount = 2;
    
    //VkPipelineDynamicStateCreateInfo dynamicState = {VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO};
    VkDynamicState dynamicStates[VK_DYNAMIC_STATE_RANGE_SIZE] = {VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR};
    dynamicState.pDynamicStates = dynamicStates;
    dynamicState.dynamicStateCount = 2;

    //  //moved to CShader
    //VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};
    //pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    //pipelineLayoutInfo.setLayoutCount = 1;
    //pipelineLayoutInfo.pSetLayouts = &shaders->descriptorSetLayout;
    //pipelineLayoutInfo.pushConstantRangeCount = 0;
    //VKERRCHECK(vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr, &pipelineLayout));
    //
    //pipelineLayout = shaders->pipelineLayout;

    VkGraphicsPipelineCreateInfo pipelineInfo = {VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO};
    pipelineInfo.stageCount = (uint32_t)shader->shaderStages.size();
    pipelineInfo.pStages              = shader->shaderStages.data();
    pipelineInfo.pVertexInputState   = &shader->vertexInputs;
    pipelineInfo.pInputAssemblyState = &inputAssembly;
    //pipelineInfo.pTessellationState = 0;
    pipelineInfo.pViewportState = &viewportState;
    pipelineInfo.pRasterizationState = &rasterizer;
    pipelineInfo.pMultisampleState = &multisampling;
    pipelineInfo.pDepthStencilState = &depthStencilState;
    pipelineInfo.pColorBlendState = &colorBlending;
    pipelineInfo.pDynamicState = &dynamicState;
    pipelineInfo.layout = shader->pipelineLayout;
    pipelineInfo.renderPass = renderpass;
    pipelineInfo.subpass = 0;
    pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
    //pipelineInfo.basePipelineIndex = 0;

    VKERRCHECK(vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &graphicsPipeline));
    return graphicsPipeline;
}
*/
