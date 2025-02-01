#include "vkray.h"

//extern PFN_vkGetRayTracingShaderGroupHandlesKHR vkGetRayTracingShaderGroupHandlesKHR;

void VKRay::Init(CQueue& queue) {
    this->gpu    = queue.gpu;
    this->device = queue.device;
    cmd.SelectQueue(queue);

#define LINK(PROC) PROC = (PFN_##PROC) vkGetDeviceProcAddr(device, #PROC);
    LINK( vkGetAccelerationStructureBuildSizesKHR      )
    LINK( vkCreateAccelerationStructureKHR             )
    LINK( vkDestroyAccelerationStructureKHR            )
    LINK( vkGetBufferDeviceAddressKHR                  )
    LINK( vkCmdBuildAccelerationStructuresKHR          )
    LINK( vkBuildAccelerationStructuresKHR             )
    LINK( vkGetAccelerationStructureDeviceAddressKHR   )
    LINK( vkCmdTraceRaysKHR                            )
//    LINK( vkGetRayTracingShaderGroupHandlesKHR         )
    LINK( vkCreateRayTracingPipelinesKHR               )
    LINK( vkCmdCopyAccelerationStructureKHR            )
    LINK( vkCmdWriteAccelerationStructuresPropertiesKHR)
#undef LINK

    blas.Init(gpu, device);
    tlas.Init(gpu, device);
}

uint32_t VKRay::AddImage(CvkImage* img) {  //return image index ()
    if(!img) return -1;
    vkImages.push_back(img);
    return (uint32_t)vkImages.size()-1;
}

void VKRay::UpdateImage(uint32_t inx, CvkImage& img) {
    //vkDeviceWaitIdle(device);
    ASSERT(inx < vkImages.size(), "UpdateImage: index out of bounds: %d", inx);
    *vkImages[inx] = std::move(img);
    ds.BindImages(6, vkImages);
    ds.UpdateSetContents();
}

uint32_t VKRay::AddMesh(VBO& vbo, IBO& ibo, UBO& ubo) {
    return blas.AddMesh(vbo,ibo,ubo, true);
}

uint32_t VKRay::AddMesh(MeshObject obj) {
    return blas.AddMesh(obj);
}

void VKRay::BuildAS() {
    blas.Build(cmd, true);
    tlas.AddInstances(blas);
    tlas.Build(cmd, false);
}

void VKRay::UpdateTLAS() {
    tlas.Build(cmd, false);
}

//-------- Descriptor Set --------
enum SHADER_STAGE {                             //  VkShaderStageFlagBits
    RGEN = VK_SHADER_STAGE_RAYGEN_BIT_KHR,      //= 0x0100
    AHIT = VK_SHADER_STAGE_ANY_HIT_BIT_KHR,     //= 0x0200
    CHIT = VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR, //= 0x0400
    MISS = VK_SHADER_STAGE_MISS_BIT_KHR,        //= 0x0800
    ALL  = RGEN|AHIT|CHIT|MISS                  //= 0x0F00
};
/*
void VKRay::CreateDescriptorSet() {
    //uint32_t imgCnt = SIZE(materials.vkImages);
    uint32_t objCnt = blas.count();

    // old
    //ds.AddBinding(0, 1,      VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR, RGEN | CHIT       );  // TLAS
    //ds.AddBinding(1, 1,      VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,              RGEN              );  // FrameBuf
    //ds.AddBinding(2, 1,      VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,             RGEN | CHIT | MISS);  // Camera
    //ds.AddBinding(3, objCnt, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,             RGEN | CHIT       );  // UBO
    //ds.AddBinding(4, objCnt, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,             RGEN | CHIT       );  // VBO
    //ds.AddBinding(5, objCnt, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,                    CHIT       );  // IBO
    //ds.AddBinding(6, imgCnt, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,     RGEN | CHIT | MISS);  // Textures
    //ds.AddBinding(7, 1,      VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,             RGEN | CHIT | MISS);  // Light
    //ds.Build(device);
    //BindTLAS(tlas);                   //0
    //BindTarget(m_target);             //1
    //BindCam(*m_camera);               //2
    //BindGeometry();                   //3,4,5
    //BindImages();                     //6
    //BindLight(*m_light);              //7
    //ds.UpdateSetContents();

    //------------DS0-----------
    ds[0].AddBinding(0, 1,      VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR, RGEN|CHIT     );  // TLAS
    ds[0].AddBinding(1, 1,      VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,              RGEN|CHIT     );  // FrameBuf
    ds[0].AddBinding(2, objCnt, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,             RGEN|CHIT|MISS);  // UBO
    ds[0].AddBinding(3, objCnt, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,                  CHIT     );  // IBO
    ds[0].AddBinding(4, objCnt, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,                  CHIT     );  // VBO
    ds[0].Build(device);
    ds[0].Bind(0, tlas);          // TLAS
    ds[0].Bind(1, m_target);      // FB
    ds[0].BindMesh(2,4,3, blas);  // Mesh
    ds[0].UpdateSetContents();
    //--------------------------
    //------------DS1-----------
    ds[1].AddBinding(0, 1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, CHIT);
    ds[1].AddBinding(1, 1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, CHIT);
    ds[1].Build(device);
    ds[1].Bind(0, materialIndexBuffer);
    ds[1].Bind(1, materialBuffer);
    ds[1].UpdateSetContents();
    //--------------------------
};
*/
//--------------------------------

void VKRay::CreateDescriptorSet() {    
    uint32_t objCnt = blas.count();
    uint32_t imgCnt = vkImages.size();

    ds.AddBinding(0, 1,      VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR, RGEN | CHIT       );  // TLAS
    ds.AddBinding(1, 1,      VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,              RGEN              );  // FrameBuf
    ds.AddBinding(2, 1,      VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,             RGEN | CHIT | MISS);  // Camera
    ds.AddBinding(3, objCnt, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,             RGEN | CHIT       );  // UBO
    ds.AddBinding(4, objCnt, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,             RGEN | CHIT       );  // VBO
    ds.AddBinding(5, objCnt, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,                    CHIT       );  // IBO
    ds.AddBinding(6, imgCnt, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,     RGEN | CHIT | MISS);  // Textures
    ds.AddBinding(7, 1,      VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,             RGEN | CHIT | MISS);  // Light
    ds.Build(device);
    ds.Bind(0, tlas);            // TLAS
    //ds.Bind(1, m_target);        // FB
    ds.Bind(2,*m_camera);        // Camera
    ds.BindMesh(3,4,5, blas);    // Mesh
    ds.BindImages(6, vkImages);  // Images
    ds.Bind(7,*m_light);         //light
    ds.UpdateSetContents();
};


void VKRay::SetRenderTarget(VkImageView target) {
    m_target = target;
    ds.Bind(1, m_target);
    ds.UpdateSetContents();
}

//---Pipeline---
/*
VkPipelineLayout VKRay::GetPipelineLayout() {
    // Collect all ds layouts into an array.
    uint cnt = ds.size();
    std::vector<VkDescriptorSetLayout> ds_layouts(cnt);
    repeat(cnt) ds_layouts[i] = ds[i].layout;

    // Combine all ds_layouts into one pipeline_layout.
    VkPipelineLayoutCreateInfo layoutCreateInfo = {VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO};
    layoutCreateInfo.setLayoutCount         = cnt;
    layoutCreateInfo.pSetLayouts            = ds_layouts.data();
    layoutCreateInfo.pushConstantRangeCount = 0;
    VkPipelineLayout layout;
    VKERRCHECK(vkCreatePipelineLayout(device, &layoutCreateInfo, NULL, &layout));
    return layout;
};
*/
VkPipelineLayout VKRay::GetPipelineLayout() {
    // Combine all ds_layouts into one pipeline_layout.
    VkPipelineLayoutCreateInfo layoutCreateInfo = {VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO};
    layoutCreateInfo.setLayoutCount         = 1;
    layoutCreateInfo.pSetLayouts            = &ds.layout;
    layoutCreateInfo.pushConstantRangeCount = 0;
    VkPipelineLayout layout;
    VKERRCHECK(vkCreatePipelineLayout(device, &layoutCreateInfo, NULL, &layout));
    return layout;
};

void VKRay::CreatePipeline() {
    pipeline.Init(gpu, device);
    pipeline.AddRayGenShader("shaders/spirv/raytrace.rgen.spv");
    pipeline.AddMissShader  ("shaders/spirv/raytrace.rmiss.spv");
    pipeline.AddMissShader  ("shaders/spirv/raytrace.2.rmiss.spv");
    pipeline.AddMissShader  ("shaders/spirv/raytrace.shadow.rmiss.spv");
    pipeline.AddHitGroup    ("shaders/spirv/raytrace.rchit.spv",0,0);
    pipeline.AddHitGroup    ("shaders/spirv/raytrace.2.rchit.spv",0,0);
    auto layout = GetPipelineLayout();
    pipeline.Create(layout);
};

//--------------

void VKRay::BindDS(VkCommandBuffer cmd) {
    //vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR, pipeline.layout, 0, 1, &ds[0].set, 0, 0);
    //vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR, pipeline.layout, 1, 1, &ds[1].set, 0, 0);

    vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR, pipeline.layout, 0, 1, &ds.set, 0, 0);    
};

void VKRay::TraceRays(VkCommandBuffer cmd, VkExtent2D ext) {
    pipeline.sbt.TraceRays(cmd, ext);
};
//--------------
