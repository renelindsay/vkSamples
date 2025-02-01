#ifndef VKRAY_H
#define VKRAY_H

#include "CDevices.h"
#include "BLAS.h"
#include "TLAS.h"
#include "Descriptor.h"
#include "RayPipeline.h"
//#include "Material.h"

class VKRay {
    VkPhysicalDevice gpu    = 0;
    VkDevice         device = 0;
    CCmd             cmd;

public:
    UBO* m_camera                = 0;
    UBO* m_light                 = 0;
    VkImageView m_target         = 0;
    //VkBuffer materialIndexBuffer = 0;  // todo: remove
    //VkBuffer materialBuffer      = 0;  // todo: remove

    //rtMaterials materials;

    std::vector<CvkImage*> vkImages{};  // TODO: move to Allocator
    //ImageList imagelist;


    BLAS blas;
    TLAS tlas;
    RayDescriptorSet ds;  // descriptor set
    //std::array<RayDescriptorSet, 2> ds;  // 2 descriptor sets


    RayPipeline pipeline;

    void Init(CQueue& queue);
    uint32_t AddImage(CvkImage* img);
    void UpdateImage(uint32_t inx, CvkImage& img);
    uint32_t AddMesh(VBO& vbo, IBO& ibo, UBO& ubo);
    uint32_t AddMesh(MeshObject obj);
    void BuildAS();
    void UpdateTLAS();

    //---- Descriptor Set ----
    void CreateDescriptorSet();
    void SetRenderTarget(VkImageView target);
    //------------------------

    //---- Pipeline ----
    VkPipelineLayout GetPipelineLayout();
    void CreatePipeline();
    //------------------
    void BindDS   (VkCommandBuffer cmd);
    void TraceRays(VkCommandBuffer cmd, VkExtent2D ext);
};


#endif
