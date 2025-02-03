#undef  _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS

#include "CShader.h"
#include "VkFormats.h"
#include "Buffers.h"
#include <algorithm>

CvkImage* CShader::imgWhite = 0;

CShader::CShader() : device(), vertShaderModule(), fragShaderModule(),
                     descriptorPool(), descriptorSetLayout(), pipelineLayout() {}

CShader::CShader(VkDevice device) : device(device), vertShaderModule(), fragShaderModule(),
                                    descriptorPool(), descriptorSetLayout(), pipelineLayout() { Init(device); }

void CShader::Init(VkDevice device) {
    this->device = device;
    if(!imgWhite) imgWhite = new CvkImage(RGBA(255,255,255,255));
    yuvSampler = default_allocator->yuv_sampler;
}

CShader::~CShader() {
    if (device) vkDeviceWaitIdle(device);
    if (imgWhite) delete imgWhite; imgWhite = 0;
    if (vertShaderModule)    vkDestroyShaderModule       (device, vertShaderModule,    nullptr);
    if (fragShaderModule)    vkDestroyShaderModule       (device, fragShaderModule,    nullptr);
    if (pipelineLayout)      vkDestroyPipelineLayout     (device, pipelineLayout,      nullptr);
    if (descriptorSetLayout) vkDestroyDescriptorSetLayout(device, descriptorSetLayout, nullptr);
    if (descriptorPool)      vkDestroyDescriptorPool     (device, descriptorPool,      nullptr);
}

bool CShader::LoadVertShader(const char* filename) {
    assert(!vertShaderModule && "Vertex shader already loaded.");
    auto spirv = LoadShader(filename);
    vertShaderModule = CreateShaderModule(spirv);
    Parse(spirv);

    // ---pack normals to 32bits---
    if((attribute_descriptions.size()==3)   // Check vertex contains 3 attributes (vnt?)
       &&(default_allocator->pack_normals)) {  // Check if pack_normals is enabled
       LOGI("Using packed normals\n");
       SetVertexAttributeFormat(1, VK_FORMAT_A2R10G10B10_SNORM_PACK32);
    }
    //-----------------------------

    return !!vertShaderModule;
}

bool CShader::LoadFragShader(const char* filename) {
    assert(!fragShaderModule && "Fragment shader already loaded.");
    auto spirv = LoadShader(filename);
    fragShaderModule = CreateShaderModule(spirv);
    Parse(spirv);
    return !!fragShaderModule;
}

std::vector<char> CShader::LoadShader(const char* filename) {
    // Read File
    printf("Load Shader: %s...", filename);
    FILE* file = fopen(filename, "rb");
    printf(" %s\n" RESET, (file?GREEN"Found" : RED"Not found"));
    assert(!!file && "File not found");

    fseek(file, 0L, SEEK_END);
    size_t file_size = (size_t) ftell(file);
    std::vector<char> buffer(file_size);
    rewind(file);
    size_t s = fread(buffer.data(), 1, file_size, file);  s=s;
    fclose(file);

    return buffer;
}

VkShaderModule CShader::CreateShaderModule(const std::vector<char>& spirv) {
    VkShaderModuleCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.codeSize = spirv.size();

    std::vector<uint32_t> codeAligned(spirv.size() / 4 + 1);
    memcpy(codeAligned.data(), spirv.data(), spirv.size());
    createInfo.pCode = codeAligned.data();

    VkShaderModule shaderModule = nullptr;
    VKERRCHECK(vkCreateShaderModule(device, &createInfo, nullptr, &shaderModule))
    return shaderModule;
}

void CShader::Parse(const std::vector<char>& spirv) {
    SpvReflectShaderModule module = {};
    SpvReflectResult result = spvReflectCreateShaderModule(spirv.size(), spirv.data(), &module);
    assert(result == SPV_REFLECT_RESULT_SUCCESS);

    //-- Enumerate DescriptorSets --
    uint32_t count = 0;
    result = spvReflectEnumerateDescriptorSets(&module, &count, nullptr);
    assert(result == SPV_REFLECT_RESULT_SUCCESS);

    std::vector<SpvReflectDescriptorSet*> sets(count);
    result = spvReflectEnumerateDescriptorSets(&module, &count, sets.data());
    assert(result == SPV_REFLECT_RESULT_SUCCESS);
    //------------------------------

    VkShaderStageFlagBits stage = (VkShaderStageFlagBits)module.shader_stage;

    VkDescriptorSet descriptorSet = nullptr;  //temp ??????

    for(auto set : sets) {
        for (uint32_t i = 0; i < set->binding_count; ++i) {
            const SpvReflectDescriptorBinding& ds_binding = *set->bindings[i];
            VkDescriptorType type = (VkDescriptorType)ds_binding.descriptor_type;

            //  Detect and merge with duplicate bindings from previous shader stages 
            for (auto& item : bindings) if (item.binding == ds_binding.binding) {
                if(item.descriptorType != (VkDescriptorType)ds_binding.descriptor_type)
                    { LOGE("Shader binding %d:\"%s\" conflicts with a previous binding.\n"     , item.binding, ds_binding.name); abort(); }  // error
                //else{ LOGV("Shader binding %d:\"%s\" referenced from multiple shader stages.\n", item.binding, ds_binding.name); }           // performance warning
                item.stageFlags |= stage;  // Mark binding as used by additional shader stage
                goto SKIP;                 // Don't add this duplicate
            }

            {   //-- VkDescriptorSetLayoutBinding array --
                VkDescriptorSetLayoutBinding layoutBinding = {};
                layoutBinding.binding         = ds_binding.binding;
                layoutBinding.descriptorType  = (VkDescriptorType)ds_binding.descriptor_type; 
                layoutBinding.descriptorCount = 1;
                for (uint32_t i_dim = 0; i_dim < ds_binding.array.dims_count; ++i_dim) {
                    layoutBinding.descriptorCount *= ds_binding.array.dims[i_dim];
                }
                layoutBinding.stageFlags         = stage;
                layoutBinding.pImmutableSamplers = nullptr; // Optional
                bindings.push_back(layoutBinding);
            }
            //  -- Descriptor Set Info --
            dsInfo.push_back({ds_binding.name, ds_binding.input_attachment_index});

            {   // -- VkWriteDescriptorSet array --
                VkWriteDescriptorSet writeDS = {VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET};
                writeDS.dstSet = descriptorSet;
                writeDS.dstBinding = ds_binding.binding; //0;
                writeDS.dstArrayElement = 0;
                writeDS.descriptorType  = type;  //VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
                writeDS.descriptorCount = 1;
                //writeDS.pBufferInfo = &bufferInfo;
                //if(type==VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER)         writeDS.pBufferInfo = &dsInfo.back().bufferInfo;
                //if(type==VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER) writeDS.pImageInfo  = &dsInfo.back().imageInfo;
                descriptorWrites.push_back(writeDS);
            }
            SKIP:;
        }
    }


    // ShaderStages
    VkPipelineShaderStageCreateInfo stageInfo = {VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO};
    stageInfo.stage = stage;  //VK_SHADER_STAGE_VERTEX_BIT;
    if(stage == VK_SHADER_STAGE_VERTEX_BIT) {
        stageInfo.module = vertShaderModule;
        vert_entry_point_name = module.entry_point_name;
        stageInfo.pName = vert_entry_point_name.c_str(); //"main";
    }
    if(stage == VK_SHADER_STAGE_FRAGMENT_BIT) {
        stageInfo.module = fragShaderModule;
        frag_entry_point_name = module.entry_point_name;
        stageInfo.pName = frag_entry_point_name.c_str(); //"main";
    }
    shaderStages.push_back(stageInfo);

#ifdef ENABLE_LOGGING
    PrintModuleInfo(module);
    for(auto& set : sets) PrintDescriptorSet(*set);
#endif

    if(stage == VK_SHADER_STAGE_VERTEX_BIT) ParseInputs(module);
    ParsePushConstants(module);

    spvReflectDestroyShaderModule(&module);
}

void CShader::ParseInputs(SpvReflectShaderModule& module) {
    uint32_t count = 0;
    SpvReflectResult result;
    result = spvReflectEnumerateInputVariables(&module, &count, nullptr);
    assert(result == SPV_REFLECT_RESULT_SUCCESS);

    std::vector<SpvReflectInterfaceVariable*> input_vars(count);
    result = spvReflectEnumerateInputVariables(&module, &count, input_vars.data());
    assert(result == SPV_REFLECT_RESULT_SUCCESS);

    // remove "gl_VertexIndex" from input_vars list
    for(uint32_t i=0; i<input_vars.size(); ++i)
        if(!strcmp(input_vars[i]->name, "gl_VertexIndex")) input_vars.erase(input_vars.begin() + i);

    // Sort attributes by location
    std::sort(std::begin(input_vars), std::end(input_vars),
      [](const SpvReflectInterfaceVariable* a, const SpvReflectInterfaceVariable* b) {
      return a->location < b->location; });

    //Populate VkPipelineVertexInputStateCreateInfo structure
    //VkVertexInputBindingDescription binding_description = {};
    binding_description = {};
    binding_description.binding = 0;
    binding_description.stride = 0;  // computed below
    binding_description.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

    vertexInputs = { VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO };
    attribute_descriptions.resize(input_vars.size());

    // Populate attribute_descriptions
    for (size_t i_var = 0; i_var < input_vars.size(); ++i_var) {
      const SpvReflectInterfaceVariable& refl_var = *(input_vars[i_var]);
      VkVertexInputAttributeDescription& attr_desc = attribute_descriptions[i_var];
      attr_desc.location = refl_var.location;
      attr_desc.binding = binding_description.binding;
      attr_desc.format = static_cast<VkFormat>(refl_var.format);
      attr_desc.offset = 0;      // final offset computed below.
    }
    ComputeVertexInputOffsets();

#ifdef ENABLE_LOGGING
    // Print input attributes
    printf("  Vertex Input attributes:\n");
    for(auto& var : input_vars) 
        printf("    %d : %s %s\n", var->location, ToStringGLSLType(*var->type_description).c_str(), var->name);
    printf("\n");
#endif
}

void CShader::ComputeVertexInputOffsets() {
    // Compute final offsets of each attribute, and total vertex stride.
    binding_description.stride = 0;
    for (auto& attribute : attribute_descriptions) {
      uint32_t format_size = FormatInfo(attribute.format).size;
      attribute.offset = binding_description.stride;
      binding_description.stride += format_size;
    }

    if(attribute_descriptions.size() > 0) {
        vertexInputs.vertexBindingDescriptionCount = 1;
        vertexInputs.pVertexBindingDescriptions = &binding_description;
        vertexInputs.vertexAttributeDescriptionCount = (uint32_t)attribute_descriptions.size();
        vertexInputs.pVertexAttributeDescriptions    =           attribute_descriptions.data();
    }
}

// Must be called after LoadVertShader() but before CreateGraphicsPipeline()
void CShader::SetVertexAttributeFormat(uint32_t location, VkFormat newFormat) {
    ASSERT(location<attribute_descriptions.size(), "SetVertexAttributeFormat: Location %d is out of range.\n", location);
    attribute_descriptions[location].format = newFormat;
    ComputeVertexInputOffsets();
}

void CShader::ParsePushConstants(SpvReflectShaderModule& module) {
    uint32_t count = 0;
    SpvReflectResult result;
    result = spvReflectEnumeratePushConstantBlocks(&module, &count, nullptr);
    assert(result == SPV_REFLECT_RESULT_SUCCESS);
    if(!count) return;

    std::vector<SpvReflectBlockVariable*> push_blocks(count);
    result = spvReflectEnumeratePushConstantBlocks(&module, &count, push_blocks.data());
    assert(result == SPV_REFLECT_RESULT_SUCCESS);

#ifdef ENABLE_LOGGING
    for(auto& block : push_blocks){
        printf("  Push Constants:  Block(%s)\n", block->name);
        for(uint32_t i=0; i<block->member_count; ++i) {
            auto& member = block->members[i];
            printf("    %d : %-12s offset=%d size=%d\n", i, member.name, member.offset, member.size);
        }
        printf("\n");
    }
#endif
}

//----DescriptorSetLayout----
VkDescriptorSetLayout& CShader::CreateDescriptorSetLayout() {
    VkDescriptorSetLayoutCreateInfo create_info = {VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO};
    create_info.bindingCount = (uint32_t)bindings.size();
    create_info.pBindings    =           bindings.data();
    VKERRCHECK( vkCreateDescriptorSetLayout(device, &create_info, nullptr, &descriptorSetLayout) )
    return descriptorSetLayout;
}

//----DescriptorPool----
VkDescriptorPool& CShader::CreateDescriptorPool(uint32_t maxSets) {
    uint32_t cnt = (uint32_t)bindings.size();
    std::vector<VkDescriptorPoolSize> poolSizes(cnt);
    for(uint32_t i = 0; i<cnt; ++i) { 
        poolSizes[i].type            = bindings[i].descriptorType;
        poolSizes[i].descriptorCount = bindings[i].descriptorCount * maxSets;
    }

    VkDescriptorPoolCreateInfo poolInfo = {VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO};
    poolInfo.maxSets       = maxSets;
    poolInfo.poolSizeCount = cnt;
    poolInfo.pPoolSizes    = poolSizes.data();
    VKERRCHECK( vkCreateDescriptorPool(device, &poolInfo, nullptr, &descriptorPool) )
    return descriptorPool;
}

//----DescriptorSet----
//--- single ds ---
VkDescriptorSet CShader::CreateDescriptorSet() {  //creates a new descriptor set
    VkDescriptorSet descriptorSet = nullptr;

    CheckBindings();
    if(!descriptorPool) CreateDescriptorPool(maxSets);
    if(!pipelineLayout) GetPipelineLayout();

    VkDescriptorSetAllocateInfo allocInfo = {VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO};
    allocInfo.descriptorPool = descriptorPool;
    allocInfo.descriptorSetCount = 1;
    allocInfo.pSetLayouts = &descriptorSetLayout;
    //VKERRCHECK( vkAllocateDescriptorSets(device, &allocInfo, &descriptorSet); )  // if out VK_ERROR_OUT_OF_POOL_MEMORY error, try increasing slots with: shader.MaxDescriptorSets()

    VkResult result = vkAllocateDescriptorSets(device, &allocInfo, &descriptorSet);
    if(result==VK_ERROR_OUT_OF_POOL_MEMORY) {LOGE("VK_ERROR_OUT_OF_POOL_MEMORY:  Try increasing shader.MaxDescriptorSets()\n");}
    VKERRCHECK(result);

    UpdateDescriptorSet(descriptorSet);
    return descriptorSet;
}

void CShader::UpdateDescriptorSet(VkDescriptorSet& descriptorSet) {  //update an existing descriptor set
    //vkDeviceWaitIdle(device);
    if(descriptorSet==0) descriptorSet = CreateDescriptorSet();
    uint32_t cnt = (uint32_t)descriptorWrites.size();
    for(uint32_t i=0; i<cnt; ++i) {
        descriptorWrites[i].dstSet      = descriptorSet;
        descriptorWrites[i].pBufferInfo = &dsInfo[i].bufferInfo;
        descriptorWrites[i].pImageInfo  = &dsInfo[i].imageInfo;
    }
    vkUpdateDescriptorSets(device, cnt, descriptorWrites.data(), 0, nullptr);
}
//-----------------
//--- ringbuffer ds ---
VkDescriptorSets CShader::CreateDescriptorSets() {  //creates a new descriptor set
    VkDescriptorSets ds;
    UpdateDescriptorSets(ds);
    return ds;
}

void CShader::UpdateDescriptorSets(VkDescriptorSets& ds) {  // update descriptorset in ringbuffer
    ds.inx = (ds.inx + 1) % ds.count;
    UpdateDescriptorSet(ds.set[ds.inx]);
};
//---------------------
//---------------------
VkPipelineLayout& CShader::GetPipelineLayout() {
    if(pipelineLayout) return pipelineLayout;
    if(!descriptorSetLayout) CreateDescriptorSetLayout();

    VkPushConstantRange pcr[] {VK_SHADER_STAGE_ALL_GRAPHICS, 0, 128};

    VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = 1;
    pipelineLayoutInfo.pSetLayouts = &descriptorSetLayout;
    pipelineLayoutInfo.pushConstantRangeCount = 1;
    pipelineLayoutInfo.pPushConstantRanges = pcr;

    VKERRCHECK(vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr, &pipelineLayout))
    return pipelineLayout;
}

void CShader::Bind(std::string name, UBO& ubo) {
    Bind(name, ubo, ubo.index);
}

void CShader::Bind(std::string name, UBO& ubo, uint32_t index) {
    ASSERT(ubo.size()>0     , "UBO has not been initialized.\n");
    ASSERT(index<ubo.Count(), "UBO array index out of bounds.\n");
    for(auto& item : dsInfo) {
        if(item.name == name) {
            //LOGI("Bind UBO   to shader var: \"%s\"\n", name.c_str())
            item.bufferInfo.buffer = ubo;
            item.bufferInfo.offset = ubo.size() * index;
            item.bufferInfo.range  = ubo.size();  //VK_WHOLE_SIZE;
            return;
        }
    } 
    LOGE("Failed to bind UBO to shader var: \"%s\" (var not found)\n", name.c_str());
}

void CShader::Bind(std::string name, VkImageView imageView, VkSampler sampler) {
    auto it = std::find(dsInfo.begin(), dsInfo.end(), name);
    if(it==dsInfo.end()){LOGE("Failed to bind Image to shader var: \"%s\" (var not found)\n", name.c_str());}

    it->imageInfo.imageView   = imageView;
    it->imageInfo.sampler     = sampler;
    it->imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
}

void CShader::Bind(std::string name, const CvkImage* image) {  // Defaults to white.
    //if(!image) //LOGW("No image bound to shader var \"%s\" : (Image is a NULL-pointer)\n", name.c_str());
    if(!image) { image = imgWhite; }  // Defaults to white.
    Bind(name, image->view, image->sampler);
}

void CShader::Bind(std::string name, const CvkImage& image) {
    auto layout = image.GetLayout();
    ASSERT(image.samples==VK_SAMPLE_COUNT_1_BIT, "Cannot bind to a multisample image.");
    switch(layout) {
        case VK_IMAGE_LAYOUT_UNDEFINED                :
        case VK_IMAGE_LAYOUT_GENERAL                  :
        case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL :
        case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL : break;
        default: LOGW("CShader::Bind : Image is not in a suitable layout.");
    }
    // For YUV textures, use immutable YUV sampler
    if(image.format==VK_FORMAT_G8B8G8R8_422_UNORM) {
        for(int i=0; i<dsInfo.size(); ++i) if(dsInfo[i].name==name) {
            bindings[i].pImmutableSamplers = &yuvSampler;
        }
    }

    Bind(name, image.view, image.sampler);
}

void CShader::Bind(uint index, const CvkImage* image) {  // Not used?
    auto& item = dsInfo[index];
    if(!image) { image = imgWhite; }  // Defaults to white.
    item.imageInfo.imageView   = image->view;
    item.imageInfo.sampler     = image->sampler;
    item.imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
}

void CShader::BindInputAttachments(std::vector<CvkImage*> attachments) {
    for(auto& item : dsInfo) {
        Bind(item.name, attachments[item.input_attachment_index]);       
        //printf("Bind Input-attachment: name=%s input_inx=%d\n", item.name.c_str(), item.input_attachment_index);
    }
}

void CShader::CheckBindings() {
    for(auto& item : dsInfo) {
        if(!item.bufferInfo.buffer) {
            LOGE("Shader item: \"%s\" was not bound. Set a binding before creating the DescriptorSet.\n", item.name.c_str());
            abort();
        }
    }
}


//---------------------------------------------PRINT---------------------------------------------
void CShader::PrintModuleInfo(const SpvReflectShaderModule& module) {
    //printf("  Source language : %s\n", spvReflectSourceLanguage(module.source_language));
    printf("  Entry Point     : %s\n", module.entry_point_name);

    const char* stage ="UNKNOWN";
    switch(module.shader_stage) {
        case SPV_REFLECT_SHADER_STAGE_VERTEX_BIT                   : stage = "VERTEX";                  break;
        case SPV_REFLECT_SHADER_STAGE_TESSELLATION_CONTROL_BIT     : stage = "TESSELLATION_CONTROL";    break;
        case SPV_REFLECT_SHADER_STAGE_TESSELLATION_EVALUATION_BIT  : stage = "TESSELLATION_EVALUATION"; break;
        case SPV_REFLECT_SHADER_STAGE_GEOMETRY_BIT                 : stage = "GEOMETRY";                break;
        case SPV_REFLECT_SHADER_STAGE_FRAGMENT_BIT                 : stage = "FRAGMENT";                break;
        case SPV_REFLECT_SHADER_STAGE_COMPUTE_BIT                  : stage = "COMPUTE";                 break;

        case SPV_REFLECT_SHADER_STAGE_TASK_BIT_NV                  : stage = "TASK";                    break;
        case SPV_REFLECT_SHADER_STAGE_MESH_BIT_NV                  : stage = "MESH";                    break;
        case SPV_REFLECT_SHADER_STAGE_RAYGEN_BIT_KHR               : stage = "RAYGEN";                  break;
        case SPV_REFLECT_SHADER_STAGE_ANY_HIT_BIT_KHR              : stage = "ANY_HIT";                 break;
        case SPV_REFLECT_SHADER_STAGE_CLOSEST_HIT_BIT_KHR          : stage = "CLOSEST_HIT";             break;
        case SPV_REFLECT_SHADER_STAGE_MISS_BIT_KHR                 : stage = "MISS";                    break;
        case SPV_REFLECT_SHADER_STAGE_INTERSECTION_BIT_KHR         : stage = "INTERSECTION";            break;
        case SPV_REFLECT_SHADER_STAGE_CALLABLE_BIT_KHR             : stage = "CALLABLE";                break;
        default : LOGE("Unknown shader stage.\n");
    }
    printf("  Shader stage    : %s\n", stage);
}

//----DescriptorSet----
void CShader::PrintDescriptorSet(const SpvReflectDescriptorSet& set) {
    printf("  Descriptor set  : %d\n", set.set);
    for (uint32_t i = 0; i < set.binding_count; ++i) {
        const SpvReflectDescriptorBinding& binding = *set.bindings[i];
        printf("       binding %2d : %-12s (%s)\n", binding.binding, binding.name, ToStringDescriptorType(binding.descriptor_type).c_str());
    }
    printf("\n");
}

std::string CShader::ToStringDescriptorType(SpvReflectDescriptorType value) {
    switch (value) {
        case SPV_REFLECT_DESCRIPTOR_TYPE_SAMPLER                    : return "VK_DESCRIPTOR_TYPE_SAMPLER";
        case SPV_REFLECT_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER     : return "VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER";
        case SPV_REFLECT_DESCRIPTOR_TYPE_SAMPLED_IMAGE              : return "VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE";
        case SPV_REFLECT_DESCRIPTOR_TYPE_STORAGE_IMAGE              : return "VK_DESCRIPTOR_TYPE_STORAGE_IMAGE";
        case SPV_REFLECT_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER       : return "VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER";
        case SPV_REFLECT_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER       : return "VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER";
        case SPV_REFLECT_DESCRIPTOR_TYPE_UNIFORM_BUFFER             : return "VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER";
        case SPV_REFLECT_DESCRIPTOR_TYPE_STORAGE_BUFFER             : return "VK_DESCRIPTOR_TYPE_STORAGE_BUFFER";
        case SPV_REFLECT_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC     : return "VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC";
        case SPV_REFLECT_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC     : return "VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC";
        case SPV_REFLECT_DESCRIPTOR_TYPE_INPUT_ATTACHMENT           : return "VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT";
        case SPV_REFLECT_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR : return "VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR";
    }
    // unhandled SpvReflectDescriptorType enum value
    return "VK_DESCRIPTOR_TYPE_???";
}

std::string CShader::ToStringGLSLType(const SpvReflectTypeDescription& type) {
  switch (type.op) {
    case SpvOpTypeVector: {
      switch (type.traits.numeric.scalar.width) {
        case 32: {
          switch (type.traits.numeric.vector.component_count) {
          case 2: return "vec2";
          case 3: return "vec3";
          case 4: return "vec4";
          }
        }
        break;

        case 64: {
          switch (type.traits.numeric.vector.component_count) {
          case 2: return "dvec2";
          case 3: return "dvec3";
          case 4: return "dvec4";
          }
        }
        break;
      }
    }
    break;

    case SpvOpTypeBool: {
        return "bool";
    }

    case SpvOpTypeInt: {
        uint32_t width = type.traits.numeric.scalar.width;
        uint32_t  sign = type.traits.numeric.scalar.signedness;
        if(width == 32) return sign ? "int" : "uint";
    }
    break;

    case SpvOpTypeFloat: {
        uint32_t width = type.traits.numeric.scalar.width;
        if(width == 32) return "float";
        if(width == 64) return "double";
    }

    default:
      break;
  }
  return "";
}

//-----------------------------------------------------------------------------------------------
