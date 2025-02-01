#include "Validation.h"
#include "CRenderpass.h"
#include "VkFormats.h"

// -----------------------------------Subpass------------------------------------
CRenderpass::CSubpass::operator VkSubpassDescription() {
    VkSubpassDescription subpass = {};
    subpass.pipelineBindPoint       = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.inputAttachmentCount    = (uint32_t)input_refs.size();
    subpass.pInputAttachments       =           input_refs.data();
    subpass.colorAttachmentCount    = (uint32_t)color_refs.size();
    subpass.pColorAttachments       =           color_refs.data();
    subpass.pResolveAttachments     =         resolve_refs.data();
    subpass.pDepthStencilAttachment = (depth_ref.layout==VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL) ? &depth_ref : nullptr;
    subpass.preserveAttachmentCount = (uint32_t)preserve_refs.size();
    subpass.pPreserveAttachments    =           preserve_refs.data();
    return subpass;
}

void CRenderpass::CSubpass::AddInputAttachment(uint32_t index) {
    if(renderpass->attachments[index].samples>1) index++; //resolved
    input_refs.push_back({index, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL});
}

void CRenderpass::CSubpass::AddColorAttachment(uint32_t index) {
    color_refs.push_back({index, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL});
    samples = renderpass->attachments[index].samples;
    if(samples>1) AddResolveAttachment(index+1);
}

//void CRenderpass::CSubpass::AddInputAttachment   (uint32_t index){input_refs  .push_back({index, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL});}
//void CRenderpass::CSubpass::AddColorAttachment   (uint32_t index){color_refs  .push_back({index, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL});}
void CRenderpass::CSubpass::AddDepthAttachment   (uint32_t index){depth_ref =            {index, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL};}
void CRenderpass::CSubpass::AddResolveAttachment (uint32_t index){resolve_refs.push_back({index, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL});}
void CRenderpass::CSubpass::AddPreserveAttachment(uint32_t index){preserve_refs.push_back(index);}
// -----------------------------------------------------------------------------

// ----------------------------------Renderpass---------------------------------
CRenderpass::CRenderpass() : device(), renderpass() {}
CRenderpass::CRenderpass(VkDevice device, uint32_t subpass_count) : device(), renderpass() { Init(device,subpass_count); }
CRenderpass::~CRenderpass() {Destroy();}
void CRenderpass::Init(VkDevice device, uint32_t subpass_count) {
    ASSERT(!this->device, "Device already selected.");
    this->device = device;
    subpasses.resize(subpass_count, this);
}

VkAttachmentDescription Attachment(VkFormat format, VkImageLayout finalLayout, VkSampleCountFlagBits samples = VK_SAMPLE_COUNT_1_BIT) {
    VkAttachmentDescription attachment = {};
    attachment.format         = format;
    attachment.samples        = samples; //VK_SAMPLE_COUNT_1_BIT;
    attachment.loadOp         = VK_ATTACHMENT_LOAD_OP_CLEAR;
    attachment.storeOp        = VK_ATTACHMENT_STORE_OP_STORE;
    attachment.stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    attachment.initialLayout  = VK_IMAGE_LAYOUT_UNDEFINED;
    attachment.finalLayout    = finalLayout; //VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
    return attachment;
}

uint32_t CRenderpass::AddAttachment(VkAttachmentDescription description, VkClearValue clearVal) {
    clearValues.push_back(clearVal);
    attachments.push_back(description);
    return (uint32_t)attachments.size()-1;
}

//--------------------------------------------------------------------------------

uint32_t CRenderpass::NewPresentAttachment(VkFormat format, VkSampleCountFlagBits samples, VkClearColorValue clearVal) {
    auto att = Attachment(format, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR, samples);
    uint inx = AddAttachment(att, *(VkClearValue*)&clearVal);
    ResolveAttachment(inx);
    return inx;
}

uint32_t CRenderpass::NewColorAttachment(VkFormat format, VkSampleCountFlagBits samples, VkClearColorValue clearVal) {
    auto att = Attachment(format, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, samples);
    uint inx = AddAttachment(att, *(VkClearValue*)&clearVal);
    ResolveAttachment(inx);
    return inx;
}

uint32_t CRenderpass::NewDepthAttachment(VkFormat format, VkSampleCountFlagBits samples, VkClearDepthStencilValue clearVal) {
    ASSERT(GetDepthAttachment()==0, "Renderpass can't have more than one depth buffer. ");
    auto attachment = Attachment(format, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, samples);
    return AddAttachment(attachment, *(VkClearValue*)&clearVal);
}

uint32_t CRenderpass::ResolveAttachment(uint32_t index) {
    auto& att = attachments[index];
    if(att.samples==1) return index;  // return if no resolve needed
    att.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    auto r_att = Attachment(att.format, att.finalLayout, VK_SAMPLE_COUNT_1_BIT);
    r_att.loadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    r_att.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    uint r_inx = AddAttachment(r_att, clearValues[index]);
    attachments[index].finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    return r_inx;
}

//--------------------------------------------------------------------------------

void CRenderpass::AddSubpassDependency(uint32_t srcSubpass, uint32_t dstSubpass) {
    VkSubpassDependency dependency = {};
    dependency.srcSubpass = srcSubpass; //VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass = dstSubpass;
    dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.srcAccessMask = 0;
    dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    dependencies.push_back(dependency);
}

VkAttachmentDescription* CRenderpass::GetPresentAttachment() {
    for(auto& att : attachments)
        if(att.finalLayout == VK_IMAGE_LAYOUT_PRESENT_SRC_KHR) return &att;
    return nullptr;
}

VkAttachmentDescription* CRenderpass::GetDepthAttachment() {
    for(auto& att : attachments)
        if(att.finalLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL) return &att;
    return nullptr;
}

std::vector<VkAttachmentDescription*> CRenderpass::GetColorAttachments() {
    std::vector<VkAttachmentDescription*> color_att;
    for(auto& att : attachments)
        if(att.finalLayout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL  ) color_att.push_back(&att);
    return color_att;
}

VkFormat CRenderpass::GetPresentFormat(){ auto* att = GetPresentAttachment(); return att?att->format:VK_FORMAT_UNDEFINED; };
VkFormat CRenderpass::GetDepthFormat()  { auto* att = GetDepthAttachment();   return att?att->format:VK_FORMAT_UNDEFINED; };

uint32_t CRenderpass::ColorAttachmentCount() {
    uint32_t att_cnt = 0;
    for(auto& att : attachments)
        if (att.finalLayout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL) ++att_cnt;
    return att_cnt;
}

void CRenderpass::Create() {
    ASSERT(device, "CRenderpass: No device selected.\n");
    ASSERT(!renderpass, "Renderpass cannot be modified after its been linked to swapchain or pipeline.\n");
    //if(renderpass) return;

    // Build subpass array
    std::vector<VkSubpassDescription> subs(subpasses.size());
    repeat(subpasses.size()) subs[i] = subpasses[i];

    VkRenderPassCreateInfo rp_info = {VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO};
    rp_info.pNext = NULL;
    rp_info.flags = 0;
    rp_info.attachmentCount = (uint32_t)attachments.size();
    rp_info.pAttachments    =           attachments.data();
    rp_info.subpassCount    = (uint32_t)subs.size();
    rp_info.pSubpasses      =           subs.data();
    rp_info.dependencyCount = (uint32_t)dependencies.size();
    rp_info.pDependencies   =           dependencies.data();
    VKERRCHECK(vkCreateRenderPass(device, &rp_info, nullptr, &renderpass));
    LOGI("Renderpass created\n");
}

void CRenderpass::Destroy() {
    if(!renderpass) return;
    vkDestroyRenderPass(device, renderpass, nullptr);
    renderpass = 0;
    LOGI("Renderpass destroyed\n");
}

//------------------PRINT-------------------
void CRenderpass::Print() {
    printf("\n");
    char buf[1024] = "RenderPass attachments:                   Subpass: ";
    repeat(subpasses.size()) strncat(buf, std::string("__"+std::to_string(i)+"__ ").c_str(), 8);
    printf("%s\n", buf);

    repeat(attachments.size()) {
        auto& att = attachments[i];
        snprintf(buf, 1024, "  %d: Format %3d: %-14s Samples: %d  Usage: ",i, att.format, FormatString(att.format).c_str(), (int)att.samples);
        for(auto& sub:subpasses) {  // Subpass Usage
            const char* typ="..... ";
            if(sub.depth_ref.layout && sub.depth_ref.attachment==i) typ="Depth ";  // Depth attachment
            for(auto& ref:sub.input_refs)     if(ref.attachment==i) typ="Input ";  // Input attachment
            for(auto& ref:sub.color_refs)     if(ref.attachment==i) typ="Color ";  // Color attachment
            for(auto& ref:sub.resolve_refs)   if(ref.attachment==i) typ="Reslv ";  // Resolve attachment
            for(auto& ref:sub.preserve_refs)  if(ref           ==i) typ="Saved ";  // Preserved
            strncat(buf, typ, 6);
        }
        if(att.finalLayout == VK_IMAGE_LAYOUT_PRESENT_SRC_KHR) strncat(buf, "(present)" ,10);
        printf("%s\n", buf);
    }
    printf("\n");
}
//------------------------------------------




/*  // Minimal RenderPass example

void CreateRenderPass(VkFormat swapchainImageFormat) {
    VkAttachmentDescription colorAttachment = {};
    colorAttachment.format         = swapchainImageFormat;
    colorAttachment.samples        = VK_SAMPLE_COUNT_1_BIT;
    colorAttachment.loadOp         = VK_ATTACHMENT_LOAD_OP_CLEAR;
    colorAttachment.storeOp        = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttachment.stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    colorAttachment.initialLayout  = VK_IMAGE_LAYOUT_UNDEFINED;
    colorAttachment.finalLayout    = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    VkAttachmentReference colorAttachmentRef = {};
    colorAttachmentRef.attachment = 0;
    colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass = {};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &colorAttachmentRef;

    VkSubpassDependency dependency = {};
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass = 0;
    dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.srcAccessMask = 0;
    dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

    VkRenderPassCreateInfo renderPassInfo = {};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.attachmentCount = 1;
    renderPassInfo.pAttachments = &colorAttachment;
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subpass;
    renderPassInfo.dependencyCount = 1;
    renderPassInfo.pDependencies = &dependency;

    VKERRCHECK(vkCreateRenderPass(device, &renderPassInfo, nullptr, &renderpass));
}
*/
