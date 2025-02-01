#ifndef SBT_H
#define SBT_H
//#include "CDevices.h"
#include "Buffers.h"

class SBT {
    VkPipeline rtPipeline = 0;
    VkStridedDeviceAddressRegionKHR m_rgenRegion{};
    VkStridedDeviceAddressRegionKHR m_missRegion{};
    VkStridedDeviceAddressRegionKHR m_hitgRegion{};
    VkStridedDeviceAddressRegionKHR m_callRegion{};
    CvkBuffer buffer;
    void Clear();
public:
    uint32_t rgen_count = 0;  // must be: 1
    uint32_t miss_count = 0;
    uint32_t hitg_count = 0;
    uint32_t call_count = 0;

    SBT()  {Clear();}
    ~SBT() {Clear();}
    void Create(VkPhysicalDevice gpu, VkDevice device, VkPipeline rtPipeline);
    void TraceRays(VkCommandBuffer cmd, VkExtent2D ext);
};

#endif
