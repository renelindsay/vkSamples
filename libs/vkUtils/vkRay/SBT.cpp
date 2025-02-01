#include "SBT.h"

void SBT::Clear() {
    buffer.Clear();
    m_rgenRegion = {};
    m_missRegion = {};
    m_hitgRegion = {};
    m_callRegion = {};
    rgen_count = 0;
    miss_count = 0;
    hitg_count = 0;
    call_count = 0;
}

//#define USE_SIMPLIFIED_ALIGNMENT  // uses 64-byte alignment for region AND handles
#ifdef  USE_SIMPLIFIED_ALIGNMENT
void SBT::Create(VkPhysicalDevice gpu, VkDevice device, VkPipeline rtPipeline) {
    ASSERT(rgen_count == 1, "No RayGen shader was added.");
    ASSERT(miss_count >= 1, "No Miss shaders were added.");
    ASSERT(hitg_count >= 1, "No HitGroup shaders were added.");

    this->rtPipeline = rtPipeline;
    //auto vkGetBufferDeviceAddressKHR          = reinterpret_cast<PFN_vkGetBufferDeviceAddressKHR>         (vkGetDeviceProcAddr(device, "vkGetBufferDeviceAddressKHR"));
    auto vkGetRayTracingShaderGroupHandlesKHR = reinterpret_cast<PFN_vkGetRayTracingShaderGroupHandlesKHR>(vkGetDeviceProcAddr(device, "vkGetRayTracingShaderGroupHandlesKHR"));

    // Get GPU memory alignment and shader handle sizes.
    VkPhysicalDeviceRayTracingPipelinePropertiesKHR raytracingProperties = {VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PIPELINE_PROPERTIES_KHR};
    VkPhysicalDeviceProperties2 props{VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2};
    props.pNext = &raytracingProperties;
    props.properties = {};
    vkGetPhysicalDeviceProperties2(gpu, &props);

    uint32_t hndCount = rgen_count + miss_count + hitg_count + call_count;
    VkDeviceSize alignment = raytracingProperties.shaderGroupBaseAlignment;  // 64
    VkDeviceSize hndSize   = raytracingProperties.shaderGroupHandleSize;     // 32
    VkDeviceSize sbtSize   = alignment * hndCount;

    VkFlags usage = VK_BUFFER_USAGE_SHADER_BINDING_TABLE_BIT_KHR |
                    VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT;

    // Copy shader handles into SBT, using 64-byte alignment
    char* hndList = new char[hndSize*hndCount];
    char* sbtData = new char[sbtSize];
    VKERRCHECK(vkGetRayTracingShaderGroupHandlesKHR(device, rtPipeline, 0, hndCount, hndSize*hndCount, hndList));
    repeat(hndCount) memcpy(sbtData + i*alignment, hndList + i*hndSize, hndSize);
    buffer.Data(sbtData, 1, sbtSize, usage, VMA_MEMORY_USAGE_CPU_TO_GPU, nullptr);
    delete[] sbtData;
    delete[] hndList;

    VkDeviceAddress addr = buffer.DeviceAddress();
    auto setRegion = [&](VkStridedDeviceAddressRegionKHR& region, uint32_t count) {
        region.deviceAddress = addr;
        region.stride        = alignment;
        region.size          = alignment * count;
        addr += region.size;
    };

    setRegion(m_rgenRegion, rgen_count);
    setRegion(m_missRegion, miss_count);
    setRegion(m_hitgRegion, hitg_count);
    setRegion(m_callRegion, call_count);
}

#else  // uses 64-byte alignment for regions BUT use 32-byte alignment for handles

constexpr uint32_t align_up(uint32_t size, uint32_t align) {
  return (size + (align - 1)) & ~(align - 1);
}

void SBT::Create(VkPhysicalDevice gpu, VkDevice device, VkPipeline rtPipeline) {
    ASSERT(rgen_count == 1, "No RayGen shader was added.");
    ASSERT(miss_count >= 1, "No Miss shaders were added.");
    ASSERT(hitg_count >= 1, "No HitGroup shaders were added.");

    this->rtPipeline = rtPipeline;
    auto vkGetRayTracingShaderGroupHandlesKHR = reinterpret_cast<PFN_vkGetRayTracingShaderGroupHandlesKHR>(vkGetDeviceProcAddr(device, "vkGetRayTracingShaderGroupHandlesKHR"));

    // Get GPU memory alignment and shader handle sizes.
    VkPhysicalDeviceRayTracingPipelinePropertiesKHR raytracingProperties = {VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PIPELINE_PROPERTIES_KHR};
    VkPhysicalDeviceProperties2 props{VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2};
    props.pNext = &raytracingProperties;
    props.properties = {};
    vkGetPhysicalDeviceProperties2(gpu, &props);
    uint32_t     hndCount = rgen_count + miss_count + hitg_count + call_count;
    VkDeviceSize baseAlignment  = raytracingProperties.shaderGroupBaseAlignment;    // 64
    VkDeviceSize hndAlignment   = raytracingProperties.shaderGroupHandleAlignment;  // 32
    VkDeviceSize hndSize        = raytracingProperties.shaderGroupHandleSize;       // 32
    VkDeviceSize handleSizeAligned = align_up(hndSize, hndAlignment);               // 32

    VkFlags usage = VK_BUFFER_USAGE_SHADER_BINDING_TABLE_BIT_KHR |
                    VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT;

    m_rgenRegion.stride        = align_up(handleSizeAligned, baseAlignment);
    m_rgenRegion.size          = align_up(handleSizeAligned, baseAlignment);
    m_missRegion.stride        = handleSizeAligned;
    m_missRegion.size          = align_up(handleSizeAligned * miss_count, baseAlignment);
    m_hitgRegion.stride        = handleSizeAligned;
    m_hitgRegion.size          = align_up(handleSizeAligned * hitg_count, baseAlignment);
    m_callRegion.stride        = handleSizeAligned;
    m_callRegion.size          = align_up(handleSizeAligned * call_count, baseAlignment);

    VkDeviceSize sbtSize = m_rgenRegion.size + m_missRegion.size + m_hitgRegion.size + m_callRegion.size;

    char* hndList = new char[hndSize*hndCount];
    char* sbtData = new char[sbtSize];
    VKERRCHECK(vkGetRayTracingShaderGroupHandlesKHR(device, rtPipeline, 0, hndCount, hndSize*hndCount, hndList));
    char* addr = sbtData;  // Host address

    int j = 0;
    repeat(rgen_count) memcpy(addr + i*handleSizeAligned, hndList + hndSize*j++, hndSize);  addr += m_rgenRegion.size;
    repeat(miss_count) memcpy(addr + i*handleSizeAligned, hndList + hndSize*j++, hndSize);  addr += m_missRegion.size;
    repeat(hitg_count) memcpy(addr + i*handleSizeAligned, hndList + hndSize*j++, hndSize);  addr += m_hitgRegion.size;
    repeat(call_count) memcpy(addr + i*handleSizeAligned, hndList + hndSize*j++, hndSize);

    buffer.Data(sbtData, 1, sbtSize, usage, VMA_MEMORY_USAGE_CPU_TO_GPU, nullptr);
    delete[] sbtData;
    delete[] hndList;

    VkDeviceAddress sbtAddress = buffer.DeviceAddress(); // device address
    m_rgenRegion.deviceAddress = sbtAddress;
    m_missRegion.deviceAddress = m_rgenRegion.deviceAddress + m_rgenRegion.size;
    m_hitgRegion.deviceAddress = m_missRegion.deviceAddress + m_missRegion.size;
}
#endif //USE_SIMPLIFIED_ALIGNMENT

void SBT::TraceRays(VkCommandBuffer cmd, VkExtent2D ext) {
    ASSERT(rtPipeline, "Raytrace Pipeline was not created yet.")
    vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR, rtPipeline);
    //vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR, rtPipelineLayout, 0, 1, &rtDescriptorSet, 0, 0);
    //vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR, rtPipelineLayout, 1, 1, &materialDescriptorSet, 0, 0);
    vkCmdTraceRaysKHR(cmd, &m_rgenRegion, &m_missRegion, &m_hitgRegion, &m_callRegion,
                      ext.width, ext.height, 1);
}


