/*
// Rene Lindsay 2019
*
*  CSHADER
*  -------
*  Use this class to load shaders, bind resources and generate descriptors.
*
*  Use the LoadVertShader/LoadFragShader functions to load SPIR-V shader files.
*  CShader uses SPIR-V reflection to examine the loaded shaders, extract the binding point names,
*  and auto-generate appropriate descriptor sets, pipeline layouts and vertex-input-attribute structs.
*  You can then use the Bind functions to bind UBO and Image resources by name.

*  Finally, call CreateDescriptorSet() to generate the following descriptor structs.
*      VkDescriptorSetLayout                   // Used by CPipeline
*      VkDescriptorPool                        // Used internally only
*      VkDescriptorSet                         // Used when you call vkCmdBindDescriptorSets
*      VkPipelineShaderStageCreateInfo         // Used by CPipeline
*      VkPipelineVertexInputStateCreateInfo    // Used by CPipeline
*
*  Use the returned VkDescriptorSet when calling vkCmdBindDescriptorSets in your renderpass.
*  The rest are used internally by the CPipeline class, when creating your pipeline.
*
*  Example:
*  --------
*    CShader shaders(device);
*    shaders.LoadVertShader("shaders/vert.spv");                        // Load Vertex shader
*    shaders.LoadFragShader("shaders/frag.spv");                        // Load Fragment shader
*    shaders.MaxDescriptorSets(7);                                      // Set how many models are using this shader
*    shaders.Bind("ubo", ubo);                                          // Bind Uniform buffer to shader binding point, named "ubo"
*    shaders.Bind("texSampler", image);                                 // Bind an Image to the shader binding point named "texSampler"
*    VkPipelineLayout pipelineLayout = shader.GetPipelineLayout();      // Generate the pipeline layout (used by vkCmdBindDescriptorSets)
*    VkDescriptorSet  descriptorSet  = shader.GetDescriptorSet();       // Generate the descriptor set  (used by vkCmdBindDescriptorSets)

*    CPipeline pipeline(device, renderpass, shaders);                   // Pass the other structs to CPipeline
*    pipeline.CreateGraphicsPipeline();
*
*
*  TODO:
*  -----
*  Add support for Tessellation, Geometry and Compute shaders
*  Add support for UBO arrays and push-constants
*
*/


#ifndef CSHADER_H
#define CSHADER_H

//#include "vkWindow.h"
#include "vkImages.h"
#include "spirv_reflect.h"

struct VkDescriptorSets { // ring-buffer the VkDescriptorSet
    static const uint32_t count = 3;
    uint32_t inx = 0;
    VkDescriptorSet set[count]{};
    operator VkDescriptorSet* () {return &set[inx];}
    operator VkDescriptorSet () {return set[inx];}
};


class CShader {
    static CvkImage* imgWhite;
//public:
    VkDevice device;
    VkShaderModule         vertShaderModule;
    VkShaderModule         fragShaderModule;
    VkDescriptorPool       descriptorPool;
    VkDescriptorSetLayout  descriptorSetLayout;
    uint32_t maxSets = 1;

    std::vector<VkDescriptorSetLayoutBinding> bindings;
    std::vector<VkWriteDescriptorSet> descriptorWrites;

    std::string vert_entry_point_name;
    std::string frag_entry_point_name;
    
    //Vertex Inputs
    VkVertexInputBindingDescription binding_description;
    std::vector<VkVertexInputAttributeDescription> attribute_descriptions;

    struct DsInfo {
        std::string name;
      uint32_t      input_attachment_index;
        union {
            VkDescriptorBufferInfo bufferInfo;
            VkDescriptorImageInfo  imageInfo;
        };
        bool operator ==(const std::string& str){return str == name;}  // for find
    };
    std::vector<DsInfo> dsInfo;

    std::vector<char> LoadShader(const char* filename);
    VkShaderModule CreateShaderModule(const std::vector<char>& spirv);
    void Parse(const std::vector<char>& spirv);
    void ParseInputs(SpvReflectShaderModule& module);
    void ParsePushConstants(SpvReflectShaderModule& module);
    void ComputeVertexInputOffsets();

    // --Print--
    void PrintModuleInfo        (const SpvReflectShaderModule& module);
    void PrintDescriptorSet     (const SpvReflectDescriptorSet& set);
    std::string ToStringDescriptorType(SpvReflectDescriptorType value);
    std::string ToStringGLSLType(const SpvReflectTypeDescription& type);
    //----------

    void CheckBindings();
    VkDescriptorSetLayout& CreateDescriptorSetLayout();
    VkDescriptorPool&  CreateDescriptorPool(uint32_t maxSets=3);
public:
    CShader();
    CShader(VkDevice device);
    ~CShader();
    void Init(VkDevice device);
    bool LoadVertShader(const char* filename);
    bool LoadFragShader(const char* filename);
    void MaxDescriptorSets(uint32_t count) {maxSets = count;}  // 3 for each mesh that uses this shader.  TODO: eliminate this?
    void SetVertexAttributeFormat(uint32_t location, VkFormat newFormat); // change a vertex input attribute's format (used to set normals to pack32)

    void Bind(std::string name, UBO& ubo);
    void Bind(std::string name, UBO& ubo, uint32_t index);
    void Bind(std::string name, VkImageView imageView, VkSampler sampler);
    void Bind(std::string name, const CvkImage* image=0);  // defaults to white
    void Bind(std::string name, const CvkImage& image);
    void Bind(uint index, const CvkImage* image);  //not used
    void BindInputAttachments(std::vector<CvkImage*> attachments);  // subpass input-attachments

    // ---used by vkCmdBindDescriptorSets --
    VkPipelineLayout& GetPipelineLayout();  
    VkDescriptorSet   CreateDescriptorSet();                  // single
    VkDescriptorSets  CreateDescriptorSets();                 // ringbuffer
    void UpdateDescriptorSet(VkDescriptorSet& descriptorSet); // update existing descriptorset (used with multi-subpass)
    void UpdateDescriptorSets(VkDescriptorSets& ds);          // update descriptorset in ringbuffer

    // ---used by CPipeline---
    VkPipelineLayout pipelineLayout;
    std::vector<VkPipelineShaderStageCreateInfo> shaderStages;
    VkPipelineVertexInputStateCreateInfo vertexInputs;

    // ---Immutable YUV Sampler---
    VkSampler yuvSampler = 0;
};

#endif
