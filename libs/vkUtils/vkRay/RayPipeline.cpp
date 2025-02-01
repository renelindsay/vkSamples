#include "RayPipeline.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

void RayPipeline::Init(VkPhysicalDevice gpu, VkDevice device) {
    this->gpu = gpu;
    this->device = device;
}

//--------------------------------------------------------------------------------------------------
VkShaderModule RayPipeline::LoadShader(const char* filename) {
    auto spirv = LoadFile(filename);
    //Parse(spirv);
    return CreateShaderModule(spirv);
}

std::vector<char> RayPipeline::LoadFile(const char* filename) {
    printf("Load Shader: %s... ", filename);
    FILE* file = fopen(filename, "rb");
    printf("%s\n", (file?"Found":"Not found"));
    assert(!!file && "File not found");

    fseek(file, 0, SEEK_END);
    size_t file_size = (size_t) ftell(file);
    std::vector<char> buffer(file_size);
    rewind(file);
    size_t s = fread(buffer.data(), 1, file_size, file); s=s;
    fclose(file);

    return buffer;
}

VkShaderModule RayPipeline::CreateShaderModule(const std::vector<char>& spirv) {
    std::vector<uint32_t> codeAligned(spirv.size() / 4 + 1);
    memcpy(codeAligned.data(), spirv.data(), spirv.size());

    VkShaderModuleCreateInfo createInfo = {VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO};
    createInfo.codeSize = spirv.size();
    createInfo.pCode = codeAligned.data();

    VkShaderModule shaderModule = nullptr;
    VKERRCHECK(vkCreateShaderModule(device, &createInfo, nullptr, &shaderModule))
    return shaderModule;
}
//--------------------------------------------------------------------------------------------------

void RayPipeline::Clear() {
    for(auto& stage : m_Stages) vkDestroyShaderModule(device, stage.module, 0);
    m_Stages.clear();
    m_Groups.clear();
}

RayPipeline::~RayPipeline() {
    Clear();
    if (pipeline) vkDestroyPipeline      (device, pipeline, nullptr);  pipeline = 0;
    if (layout)   vkDestroyPipelineLayout(device, layout,   nullptr);  layout = 0;
};

//-----------------------------------------------------------

uint32_t RayPipeline::AddStage(VkShaderModule module, VkShaderStageFlagBits shader_stage) {
    auto& stage  = m_Stages.emplace_back();
    stage = {VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO};
    stage.stage  = shader_stage;
    stage.module = module;
    stage.pName  = "main";
    return m_Stages.size()-1;
}

uint32_t RayPipeline::AddGeneral(VkShaderModule module, VkShaderStageFlagBits shader_stage) {
    assert(shader_stage & (VK_SHADER_STAGE_RAYGEN_BIT_KHR | VK_SHADER_STAGE_MISS_BIT_KHR | VK_SHADER_STAGE_CALLABLE_BIT_KHR ));
    auto& group = m_Groups.emplace_back();
    group = {VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_KHR};
    group.type               = VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_KHR;
    group.generalShader      = AddStage(module, shader_stage);
    group.closestHitShader   = VK_SHADER_UNUSED_KHR;
    group.anyHitShader       = VK_SHADER_UNUSED_KHR;
    group.intersectionShader = VK_SHADER_UNUSED_KHR;
    return m_Groups.size()-1;
}

uint32_t RayPipeline::AddHitGroup(VkShaderModule closehit, VkShaderModule anyhit, VkShaderModule intersect) {
    auto& group = m_Groups.emplace_back();
    group = {VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_KHR};
    group.type = intersect ? VK_RAY_TRACING_SHADER_GROUP_TYPE_PROCEDURAL_HIT_GROUP_KHR
                           : VK_RAY_TRACING_SHADER_GROUP_TYPE_TRIANGLES_HIT_GROUP_KHR;
    group.generalShader      = VK_SHADER_UNUSED_KHR;
    group.closestHitShader   = closehit  ? AddStage(closehit,  VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR)  : VK_SHADER_UNUSED_KHR;
    group.anyHitShader       = anyhit    ? AddStage(anyhit,    VK_SHADER_STAGE_ANY_HIT_BIT_KHR)      : VK_SHADER_UNUSED_KHR;
    group.intersectionShader = intersect ? AddStage(intersect, VK_SHADER_STAGE_INTERSECTION_BIT_KHR) : VK_SHADER_UNUSED_KHR;
    return m_Groups.size()-1;
}

//-----------------------------------------------------------

uint32_t RayPipeline::AddRayGenShader(const char* filename) {
    ASSERT(m_Groups.size()==0, "Only one RayGen shader is allowed, and must be added first.");
    ++sbt.rgen_count;
    VkShaderModule rgen = LoadShader(filename);
    return AddGeneral(rgen, VK_SHADER_STAGE_RAYGEN_BIT_KHR);
}

uint32_t RayPipeline::AddMissShader(const char* filename) {
    ASSERT(m_Groups.size()>0, "The first shader MUST be the RayGen shader.");
    ASSERT(sbt.hitg_count==0, "Miss shaders must be added before hit-group shaders.");
    ++sbt.miss_count;
    VkShaderModule miss = LoadShader(filename);
    return AddGeneral(miss, VK_SHADER_STAGE_MISS_BIT_KHR);
}

uint32_t RayPipeline::AddHitGroup(const char* closehit,
                                  const char* anyhit,
                                  const char* intersect) {
    ASSERT(m_Groups.size()>0, "The first shader MUST be the RayGen shader.");
    ASSERT(m_Groups.size()>1, "Add Miss shaders before hit-group shaders.");
    ++sbt.hitg_count;
    VkShaderModule chit  = closehit  ? LoadShader(closehit)  : 0;
    VkShaderModule ahit  = anyhit    ? LoadShader(anyhit)    : 0;
    VkShaderModule isect = intersect ? LoadShader(intersect) : 0;
    return AddHitGroup(chit, ahit, isect);
}

uint32_t RayPipeline::AddCallShader(const char* filename) {
    ASSERT(m_Groups.size()>0, "The first shader MUST be the RayGen shader.");
    ASSERT(sbt.hitg_count>0, "Miss hit-group shaders must be added before callable shaders.");
    ++sbt.miss_count;
    VkShaderModule miss = LoadShader(filename);
    return AddGeneral(miss, VK_SHADER_STAGE_CALLABLE_BIT_KHR);
}

void RayPipeline::Create(VkPipelineLayout layout, uint32_t maxRecursionDepth) {
    this->layout = layout;
    VkRayTracingPipelineCreateInfoKHR rayPipelineCreateInfo = {VK_STRUCTURE_TYPE_RAY_TRACING_PIPELINE_CREATE_INFO_KHR};
    rayPipelineCreateInfo.flags = 0;
    rayPipelineCreateInfo.stageCount = m_Stages.size();  // 4
    rayPipelineCreateInfo.pStages    = m_Stages.data();
    rayPipelineCreateInfo.groupCount = m_Groups.size();  // 4
    rayPipelineCreateInfo.pGroups    = m_Groups.data();
    rayPipelineCreateInfo.maxPipelineRayRecursionDepth = maxRecursionDepth;
    rayPipelineCreateInfo.layout = layout;
    rayPipelineCreateInfo.basePipelineHandle = VK_NULL_HANDLE;
    rayPipelineCreateInfo.basePipelineIndex = -1;

    VKERRCHECK(vkCreateRayTracingPipelinesKHR(device, VK_NULL_HANDLE, VK_NULL_HANDLE, 1,
                                              &rayPipelineCreateInfo, NULL, &pipeline));
    sbt.Create(gpu, device, pipeline);
    Clear();
}
