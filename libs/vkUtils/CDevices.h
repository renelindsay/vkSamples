// Copyright (c) 2017 Rene Lindsay

/*
*  This unit wraps Physical devices, Logical devices and Queues.
*  Use CPhysicalDevice.CreateDevice(), to create a logical device, with queues.
*
*  └CPhysicalDevices[]
*   └CPhysicalDevice ----------------------------------> : CDevice
*     ├VkPhysicalDeviceProperties                          └CQueue[]
*     ├VkPhysicalDeviceFeatures
*     ├CDeviceExtensions[]        (Picklist)
*     └CQueueFamily[]             (array)
*
*  WARNING: This unit is a work in progress.
*  Interfaces are experimental and likely to change.
*
* CPhysicalDevices:  // TODO: Make this a member of CInstance
* -----------------
* Create an instance of CPhysicalDevices, to enumerate the available GPUs, and their properties.
* Use the FindPresentable() function to find which GPU can present to the given window surface.

* CPhysicalDevice:
* ----------------
* Use FindSurfaceFormat() function to find a supported color format for the given window surface.
* Use FindDepthFormat() function to find a supported depth format.
*
* CDevice:
* --------
* Create an instance of CDevice, using the picked CPhysicalDevice as input.
* Then call the AddQueue() function, to add queues to the logical device.
*
*/

#ifndef CDEVICES_H
#define CDEVICES_H

#include "CInstance.h"
#include "WindowBase.h"

//------------------------CPhysicalDevice-------------------------
class CPhysicalDevice {
  public:
    CPhysicalDevice();
    const char* VendorName() const;
    const char* APIVersion() const;
    // -- Read-only properties --
    VkPhysicalDevice                 handle;
    VkPhysicalDeviceProperties       properties;      // properties and limits
    VkPhysicalDeviceFeatures         features;        // list of available features  // Vulkan 1.0

    VkPhysicalDeviceFeatures2                   features2;  // Vulkan 1.2
    VkPhysicalDeviceVulkan11Features            features_11;
    VkPhysicalDeviceVulkan12Features            features_12;
    //VkPhysicalDeviceDescriptorIndexingFeatures  features_DescriptorIndex;
    //VkPhysicalDeviceBufferDeviceAddressFeatures features_DeviceAddress;
    VkPhysicalDeviceAccelerationStructureFeaturesKHR features_AccelerationStructure;
    VkPhysicalDeviceRayTracingPipelineFeaturesKHR    features_RayTracingPipeline;
    VkPhysicalDeviceRayQueryFeaturesKHR              features_RayQuery;

    std::vector<VkQueueFamilyProperties> queue_families;  // array of queue families
    // VkSurfaceCapabilitiesKHR   surface_caps;
    // -- Configurable properties --
    CDeviceExtensions                                extensions;             // picklist: select extensions to load (Defaults to "VK_KHR_swapchain" only.)
    VkPhysicalDeviceFeatures                         enable_features  = {};  // Set required features.   TODO: finish this.
    VkPhysicalDeviceVulkan11Features                 enable_features_11={};
    VkPhysicalDeviceVulkan12Features                 enable_features_12={};
    VkPhysicalDeviceAccelerationStructureFeaturesKHR enable_features_AccelerationStructure={};
    VkPhysicalDeviceRayQueryFeaturesKHR              enable_features_RayQuery={};
    VkPhysicalDeviceRayTracingPipelineFeaturesKHR    enable_features_RayTracingPipeline={};

    operator VkPhysicalDevice() const { return handle; }
    int FindQueueFamily(VkQueueFlags flags, VkSurfaceKHR surface = 0);  // Returns a QueueFamlyIndex, or -1 if none found.

    std::vector<VkSurfaceFormatKHR> SurfaceFormats(VkSurfaceKHR surface);     // Returns list of supported surface formats.
    VkFormat FindSurfaceFormat(VkSurfaceKHR surface,                          // Returns first supported format from given list, or VK_FORMAT_UNDEFINED if no match was found.
        std::vector<VkFormat> preferred_formats = {VK_FORMAT_R8G8B8A8_SRGB,
                                                   VK_FORMAT_B8G8R8A8_SRGB,
                                                   VK_FORMAT_R8G8B8A8_UNORM,
                                                   VK_FORMAT_B8G8R8A8_UNORM});
    VkFormat FindDepthFormat(
        std::vector<VkFormat> preferred_formats = {VK_FORMAT_D32_SFLOAT,          // Returns first supported depth format from list,
                                                   VK_FORMAT_D32_SFLOAT_S8_UINT,  // or VK_FORMAT_UNDEFINED if no match was found.
                                                   VK_FORMAT_X8_D24_UNORM_PACK32,
                                                   VK_FORMAT_D24_UNORM_S8_UINT,
                                                   VK_FORMAT_D16_UNORM_S8_UINT,
                                                   VK_FORMAT_D16_UNORM});
};
//----------------------------------------------------------------
//------------------------CPhysicalDevices------------------------
class CPhysicalDevices {
    std::vector<CPhysicalDevice> gpu_list;

   public:
    CPhysicalDevices();
    CPhysicalDevices(const VkInstance instance);
    void Init(const VkInstance instance);
    uint32_t Count() { return (uint32_t)gpu_list.size(); }
    CPhysicalDevice* FindPresentable(VkSurfaceKHR surface);  // Returns first device able to present to surface, or null if none.
    CPhysicalDevice& operator[](const int i) { return gpu_list[i]; }
    void Print(bool show_queues = false);
};
//----------------------------------------------------------------
//-----------------------------CQueue-----------------------------
struct CQueue {
    VkQueue         queue;
    uint32_t        family;   // queue family
    uint32_t        index;    // queue index
    VkQueueFlags    flags;    // Graphics / Compute / Transfer / Sparse / Protected
    VkSurfaceKHR    surface;  // 0 if queue can not present
    VkDevice        device;   // (used by Swapchain)
    CPhysicalDevice gpu;      // (used by Swapchain)

    VkCommandPool   CreateCommandPool() const;
    VkCommandBuffer CreateCommandBuffer(VkCommandPool command_pool) const;
    void WaitIdle(){vkQueueWaitIdle(queue);}

    operator VkQueue() const {
        ASSERT(!!queue, "Queue not yet initialized. ");
        return queue;
    }
};
//----------------------------------------------------------------
//-----------------------------CDevice----------------------------
class CDevice {
    //friend class CSwapchain;
    //friend class CAllocator;
    VkDevice            handle=0;
    CPhysicalDevice     gpu;
    std::vector<CQueue> queues;
    uint FamilyQueueCount(uint family);
  public:
    CDevice(CPhysicalDevice gpu);
    CDevice(){}
    ~CDevice();
    void SelectGPU(CPhysicalDevice gpu);
    void Create();  //Select GPU and add queues BEFORE calling create
    void Destroy();
    CQueue* AddQueue(VkQueueFlags flags, VkSurfaceKHR surface = nullptr);  // returns 0 if failed
    void WaitIdle(){vkDeviceWaitIdle(handle);}
    operator VkDevice() {
        if(!handle) Create();
        return handle; 
    }
};
//----------------------------------------------------------------
//------------------------------CCmd------------------------------ // Command Buffer
class CCmd {                                                       // TODO: merge with CQueue?
  public:
  //friend class CBuffers;
    VkDevice        device;
    VkQueue         queue;
    VkCommandPool   command_pool;
    VkCommandBuffer command_buffer;
    VkFence         fence;
    bool            recording=false;
  public:
    CCmd(const CQueue& queue);
    CCmd();
    ~CCmd();
    void SelectQueue(const CQueue& queue);
    void Begin(VkCommandBufferUsageFlags flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
    void End(bool submit = false);
    void Submit(bool wait = true);  // (TODO: Default to false?)
    void Wait();  //wait for submit to complete
    operator VkCommandBuffer() {return command_buffer;}
};
//----------------------------------------------------------------

// ----Vulkan cmd----
static void vkCmdBindVertexBuffer(VkCommandBuffer cmd, const VkBuffer* vbo) {
    VkDeviceSize offset = 0;
    vkCmdBindVertexBuffers(cmd, 0, 1, vbo, &offset);
}

static void vkCmdClearDepthStencil(VkCommandBuffer cmd, VkExtent2D extent) {
    VkClearAttachment ca{};
    ca.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
    ca.clearValue.depthStencil = {1.f, 0};
    VkClearRect cr;
    cr.rect.offset = {0,0};
    cr.rect.extent = extent;
    cr.layerCount = 1;
    cr.baseArrayLayer = 0;
    vkCmdClearAttachments(cmd, 1, &ca, 1, &cr);
}
//-------------------


#endif
