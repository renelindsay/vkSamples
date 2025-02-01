#ifndef VKRAY_HELPERS_H
#define VKRAY_HELPERS_H

#include "CDevices.h"
#include <map>

//-------------------------------------------HELPERS-------------------------------------------------
//------------------------DeviceAddress-----------------------
struct DeviceAddress {
    DeviceAddress(VkDeviceAddress a) : deviceAddress(a) {}
    VkDeviceAddress deviceAddress;
    operator VkDeviceOrHostAddressConstKHR () {return *(VkDeviceOrHostAddressConstKHR*)this;}
    operator VkDeviceOrHostAddressKHR      () {return *(VkDeviceOrHostAddressKHR*)this;}
    operator VkDeviceAddress               () {return *(VkDeviceAddress*)this;}
};

static DeviceAddress deviceAddress(VkDevice device, VkBuffer vkBuf) {
    VkBufferDeviceAddressInfo bufInfo {VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO, 0, vkBuf};
    return vkGetBufferDeviceAddress(device, &bufInfo);
}
//------------------------------------------------------------

static void mem_barrier(VkCommandBuffer cmd, VkAccessFlags src, VkAccessFlags dst) {
    static const std::map<VkAccessFlags, VkPipelineStageFlags> stages = {  // what, when
        {VK_ACCESS_TRANSFER_WRITE_BIT,                   VK_PIPELINE_STAGE_TRANSFER_BIT},
        {VK_ACCESS_ACCELERATION_STRUCTURE_WRITE_BIT_KHR, VK_PIPELINE_STAGE_ACCELERATION_STRUCTURE_BUILD_BIT_KHR},
        {VK_ACCESS_ACCELERATION_STRUCTURE_READ_BIT_KHR,  VK_PIPELINE_STAGE_ACCELERATION_STRUCTURE_BUILD_BIT_KHR},
    };
    VkMemoryBarrier barrier{VK_STRUCTURE_TYPE_MEMORY_BARRIER};
    barrier.srcAccessMask = src;
    barrier.dstAccessMask = dst;
    auto srcStage = stages.find(src);  if(srcStage == stages.end()) { LOGE("Unknown src barier mapping"); }
    auto dstStage = stages.find(dst);  if(dstStage == stages.end()) { LOGE("Unknown dst barier mapping"); }
    vkCmdPipelineBarrier(cmd, srcStage->second, dstStage->second, 0, 1, &barrier, 0, nullptr, 0, nullptr);
}

//-------------------------------------------------------------------------------------------------

#endif
