#ifndef DESCRIPTORSET_H
#define DESCRIPTORSET_H

#include "BLAS.h"
#include "vkImages.h"

//--------------------------------------------WriteInfo---------------------------------------------
template <typename T> class WriteInfo {
    std::vector<VkWriteDescriptorSet> writeDS;
    std::vector<std::vector<T>>       contents;
    void SetImgPointers() { repeat(count()){ writeDS[i].pImageInfo  = contents[i].data(); }}
    void SetBufPointers() { repeat(count()){ writeDS[i].pBufferInfo = contents[i].data(); }}
    void SetASPointers()  { repeat(count()){ writeDS[i].pNext       = contents[i].data(); }}

public:
    uint32_t count(){ return writeDS.size();}

    void Bind(VkDescriptorSet set, uint32_t binding, VkDescriptorType type, const std::vector<T>& info) {
        VkWriteDescriptorSet descriptorWrite = {VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET};
        descriptorWrite.dstSet               = set;
        descriptorWrite.dstBinding           = binding;
        descriptorWrite.dstArrayElement      = 0;
        descriptorWrite.descriptorType       = type;
        descriptorWrite.descriptorCount      = (uint32_t)info.size();
        descriptorWrite.pBufferInfo          = VK_NULL_HANDLE;
        descriptorWrite.pImageInfo           = VK_NULL_HANDLE;
        descriptorWrite.pTexelBufferView     = VK_NULL_HANDLE;
        descriptorWrite.pNext                = VK_NULL_HANDLE;

        // If Bind point was already added, update instead. (eg. for window resize)
        for(size_t i = 0; i < writeDS.size(); i++) {
            if(writeDS[i].dstBinding == binding) {
                writeDS[i] = descriptorWrite;
                contents[i] = info;
                return;
            }
        }

        writeDS.push_back(descriptorWrite);
        contents.push_back(info);
    }

    auto imgData() { SetImgPointers();  return writeDS.data(); }
    auto bufData() { SetBufPointers();  return writeDS.data(); }
    auto asData () { SetASPointers();   return writeDS.data(); }
};
//--------------------------------------------------------------------------------------------------

//-----------------------------------------RayDescriptorSet-----------------------------------------
typedef std::map<uint32_t, VkDescriptorSetLayoutBinding> BindingMap;

class RayDescriptorSet {
    VkDevice         device=0;
    BindingMap       bindings;  // Associate binding slot index with the binding info

    WriteInfo<VkDescriptorBufferInfo>                       buffers;
    WriteInfo<VkDescriptorImageInfo>                        images;
    WriteInfo<VkWriteDescriptorSetAccelerationStructureKHR> structures;

    VkDescriptorPool      CreatePool(uint32_t maxSets = 1);
    VkDescriptorSetLayout CreateLayout();
    VkDescriptorSet       CreateSet(VkDescriptorPool pool, VkDescriptorSetLayout layout);

public:
    VkDescriptorPool      pool  =0;
    VkDescriptorSetLayout layout=0;
    VkDescriptorSet       set   =0;

    RayDescriptorSet(){}
    ~RayDescriptorSet();

    void AddBinding(
        uint32_t           binding,           //  Binding slot (shader layout index)
        uint32_t           count,             //  descriptor count (for arrays)
        VkDescriptorType   type,              //  descriptor type
        VkShaderStageFlags stage,             //  shader stages that uses this resource
        VkSampler*         sampler = nullptr  //  for materials?  (remove?)
    );

    void Build(VkDevice device);  //after addBinding is done
    // Binding functions
    void Bind(uint32_t binding, const std::vector<VkDescriptorBufferInfo>& bufferInfo);                      // Bind buffers
    void Bind(uint32_t binding, const std::vector<VkDescriptorImageInfo>& imageInfo);                        // Bind images
    void Bind(uint32_t binding, const std::vector<VkWriteDescriptorSetAccelerationStructureKHR>& accelInfo); // Bind AS
    // Additional convenience binding functions
    void Bind(uint32_t binding, VkAccelerationStructureKHR& as);                                  // Bind TLAS
    void Bind(uint32_t binding, VkImageView view);                                                // Bind RenderTarget
    void Bind(uint32_t binding, VkBuffer buffer);                                                 // Bind UBO
    void BindMesh(uint32_t ubo_bind, uint32_t vbo_bind, uint32_t ibo_bind, MeshList& mesh_list);  // Bind Geometries
    void BindImages(uint32_t binding, std::vector<CvkImage*>& img);                               // Bind Textures

    void UpdateSetContents();
};
//--------------------------------------------------------------------------------------------------
#endif
