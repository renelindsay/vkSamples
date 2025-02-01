#ifndef RAYPIPELINE_H
#define RAYPIPELINE_H

#include "Validation.h"
#include <vector>
#include "SBT.h"

class RayPipeline {
    VkPhysicalDevice gpu = 0;
    VkDevice      device = 0;

    VkShaderModule    LoadShader(const char* filename);
    std::vector<char> LoadFile(const char* filename);
    VkShaderModule    CreateShaderModule(const std::vector<char>& spirv);

    std::vector<VkPipelineShaderStageCreateInfo>      m_Stages;
    std::vector<VkRayTracingShaderGroupCreateInfoKHR> m_Groups;
    uint32_t AddStage(VkShaderModule module, VkShaderStageFlagBits shader_stage);
    uint32_t AddGeneral(VkShaderModule module, VkShaderStageFlagBits shader_stage);
    uint32_t AddHitGroup(VkShaderModule chit, VkShaderModule ahit=0, VkShaderModule intersect=0);
    VkPipeline pipeline = 0;
public:

    SBT sbt;
    VkPipelineLayout layout = 0;

    RayPipeline(){};
    ~RayPipeline();

    void Init(VkPhysicalDevice gpu, VkDevice device);
    void Clear();

    uint32_t AddRayGenShader(const char* filename);    // Only one RayGen shader allowed, and must be first.
    uint32_t AddMissShader  (const char* filename);    // One or more miss shaders may be added.
    uint32_t AddHitGroup    (const char* closehit,     // One or more hitGroups may be added.
                             const char* anyhit   =0,  // Any-hit shader is Optional.
                             const char* intersect=0); // Intersection shader is Optional.
    uint32_t AddCallShader  (const char* filename);    // Callable shader is optional.

    void Create(VkPipelineLayout layout, uint32_t maxRecursionDepth = 3);
    //operator VkPipeline() const { return m_Pipeline; }
};


#endif
