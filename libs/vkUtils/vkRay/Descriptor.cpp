#include "Descriptor.h"

RayDescriptorSet::~RayDescriptorSet() {
    if(layout) vkDestroyDescriptorSetLayout(device, layout, nullptr);  layout = VK_NULL_HANDLE;
    if(pool)   vkDestroyDescriptorPool     (device,   pool, nullptr);  pool   = VK_NULL_HANDLE;
}

//--------------------------------------------------------------------------------------------------
void RayDescriptorSet::AddBinding(uint32_t binding, uint32_t count, VkDescriptorType type, VkShaderStageFlags stage, VkSampler* sampler) {
    if(!count) return;
    VkDescriptorSetLayoutBinding b = {};
    b.binding                      = binding;
    b.descriptorType               = type;
    b.descriptorCount              = count;
    b.stageFlags                   = stage;
    b.pImmutableSamplers           = sampler;
    if(bindings.find(binding) != bindings.end()){ LOGE("Descriptor set binding already used."); }  //prevent reusing same binding point
    bindings[binding] = b;
}
//--------------------------------------------------------------------------------------------------

void RayDescriptorSet::Build(VkDevice device) {
    this->device = device;
    pool   = CreatePool();
    layout = CreateLayout();
    set    = CreateSet(pool, layout);
}
//--------------------------------------------------------------------------------------------------

VkDescriptorPool RayDescriptorSet::CreatePool(uint32_t maxSets) {
    if(pool) return pool;
    std::vector<VkDescriptorPoolSize> counters;
    counters.reserve(bindings.size());

    for(const auto& b : bindings) {
      counters.push_back({b.second.descriptorType, b.second.descriptorCount});
    }

    VkDescriptorPoolCreateInfo poolInfo = {};
    poolInfo.sType                      = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.poolSizeCount              = static_cast<uint32_t>(counters.size());
    poolInfo.pPoolSizes                 = counters.data();
    poolInfo.maxSets                    = maxSets;
    //VkDescriptorPool pool=0;
    VKERRCHECK(vkCreateDescriptorPool(device, &poolInfo, nullptr, &pool));
    return pool;
}
//--------------------------------------------------------------------------------------------------
// Generates the descriptor layout corresponding to the bound resources
VkDescriptorSetLayout  RayDescriptorSet::CreateLayout() {
    std::vector<VkDescriptorSetLayoutBinding> bindingList;  // Todo: avoid this copy
    bindingList.reserve(bindings.size());
    for(const auto& b : bindings) {
        bindingList.push_back(b.second);
    }

    // Create the layout from the vector of bindings
    VkDescriptorSetLayoutCreateInfo layoutInfo = {VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO};
    layoutInfo.bindingCount                    = bindings.size();
    layoutInfo.pBindings                       = bindingList.data();
    VkDescriptorSetLayout layout = 0;
    VKERRCHECK(vkCreateDescriptorSetLayout(device, &layoutInfo, nullptr, &layout));
    return layout;
}
//--------------------------------------------------------------------------------------------------
// Generate a descriptor set from the pool and layout
VkDescriptorSet RayDescriptorSet::CreateSet(VkDescriptorPool pool, VkDescriptorSetLayout layout) {
    VkDescriptorSetAllocateInfo allocInfo = {VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO};
    allocInfo.descriptorPool              = pool;
    allocInfo.descriptorSetCount          = 1;
    allocInfo.pSetLayouts                 = &layout;
    VkDescriptorSet set = 0;
    VKERRCHECK(vkAllocateDescriptorSets(device, &allocInfo, &set));
    return set;
}
//--------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------
// Bind buffers
void RayDescriptorSet::Bind(uint32_t binding, const std::vector<VkDescriptorBufferInfo>& bufferInfo) {
    buffers.Bind(set, binding, bindings[binding].descriptorType, bufferInfo);
}

// Bind images
void RayDescriptorSet::Bind(uint32_t binding, const std::vector<VkDescriptorImageInfo>& imageInfo) {
    images.Bind(set, binding, bindings[binding].descriptorType, imageInfo);
}

// Bind an acceleration structures
void RayDescriptorSet::Bind(uint32_t binding, const std::vector<VkWriteDescriptorSetAccelerationStructureKHR>& accelInfo) {
    structures.Bind(set, binding, bindings[binding].descriptorType, accelInfo);
}

// Additional convenience binding functions
//==================================================================================================
void RayDescriptorSet::Bind(uint32_t binding, VkAccelerationStructureKHR& as) {  // TLAS
    Bind(binding, {{VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET_ACCELERATION_STRUCTURE_KHR, 0, 1, &as}});
}

void RayDescriptorSet::Bind(uint32_t binding, VkImageView view) {  // RenderTarget
     Bind(binding, {{0, view, VK_IMAGE_LAYOUT_GENERAL}});
}

void RayDescriptorSet::Bind(uint32_t binding, VkBuffer buffer) { // Bind a buffer (UBO)
    Bind(binding, {{buffer, 0, VK_WHOLE_SIZE}});
}

void RayDescriptorSet::BindMesh(uint32_t ubo_bind, uint32_t vbo_bind, uint32_t ibo_bind, MeshList& mesh_list) {  // Bind Mesh list
    uint cnt = mesh_list.size();
    std::vector<VkDescriptorBufferInfo> uboInfo(cnt);
    std::vector<VkDescriptorBufferInfo> vboInfo(cnt);
    std::vector<VkDescriptorBufferInfo> iboInfo(cnt);
    repeat(cnt) {
       uboInfo[i] = {mesh_list[i].uniformBuffer, 0, VK_WHOLE_SIZE};
       vboInfo[i] = {mesh_list[i].vertexBuffer,  0, VK_WHOLE_SIZE};
       iboInfo[i] = {mesh_list[i].indexBuffer,   0, VK_WHOLE_SIZE};
    }
    Bind(ubo_bind, uboInfo);
    Bind(vbo_bind, vboInfo);
    Bind(ibo_bind, iboInfo);
}

void RayDescriptorSet::BindImages(uint32_t binding, std::vector<CvkImage*>& img_list) {  // Bind Image list
    uint32_t cnt = img_list.size();
    std::vector<VkDescriptorImageInfo> info(cnt);
    repeat(cnt) info[i] = {img_list[i]->sampler, img_list[i]->view, img_list[i]->GetLayout()};
    if(cnt) Bind(binding, info);
}

//==================================================================================================

//--------------------------------------------------------------------------------------------------
// For each resource type, set the actual pointers in the VkWriteDescriptorSet structures,
// and write the resulting structures into the descriptor set
void RayDescriptorSet::UpdateSetContents() {
  ASSERT(set, "Descriptor must be built before it can be updated.");
  vkUpdateDescriptorSets(device, buffers.count(),    buffers  .bufData(), 0, nullptr);
  vkUpdateDescriptorSets(device, images.count(),     images   .imgData(), 0, nullptr);
  vkUpdateDescriptorSets(device, structures.count(), structures.asData(), 0, nullptr);
}
//--------------------------------------------------------------------------------------------------


