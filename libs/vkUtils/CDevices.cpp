// Copyright (c) 2017 Rene Lindsay

#include "CDevices.h"

//------------------------CPhysicalDevice-------------------------
CPhysicalDevice::CPhysicalDevice() : handle(0), properties(), features(), features2(), extensions() {}

const char* CPhysicalDevice::VendorName() const {
    struct {const uint id; const char* name;} vendors[] =
    {{0x1002, "AMD"}, {0x10DE, "NVIDIA"}, {0x8086, "INTEL"}, {0x13B5, "ARM"}, {0x5143, "Qualcomm"}, {0x1010, "Imagination"}};
    for (auto vendor : vendors) if (vendor.id == properties.vendorID) return vendor.name;
    return "";
}

//struct version{ uint32_t major, minor, patch; };
const char* CPhysicalDevice::APIVersion() const {
    uint32_t ver = properties.apiVersion;
    static char buf[16]{};
    sprintf(buf, "%d.%d.%d", ver>>22, ver>>12&0x3ff, ver&0xfff);
    return buf;
}

// Find queue-family with requred flags, and can present to given surface. (if provided)
// Returns the QueueFamily index, or -1 if not found.
int CPhysicalDevice::FindQueueFamily(VkQueueFlags flags, VkSurfaceKHR surface){
    repeat (queue_families.size()) {
        VkBool32 can_present = VK_FALSE;
        if ((queue_families[i].queueFlags & flags) != flags) continue;
        if (surface) VKERRCHECK(vkGetPhysicalDeviceSurfaceSupportKHR(handle, i, surface, &can_present));
        if (!!surface == !!can_present) return i;
    }
    return -1;
}

std::vector<VkSurfaceFormatKHR> CPhysicalDevice::SurfaceFormats(VkSurfaceKHR surface) {  // Get Surface format list
    std::vector<VkSurfaceFormatKHR> formats;
    uint32_t count = 0;
    vkGetPhysicalDeviceSurfaceFormatsKHR(handle, surface, &count, nullptr);
    formats.resize(count);
    vkGetPhysicalDeviceSurfaceFormatsKHR(handle, surface, &count, formats.data());
    ASSERT(!!count, "No supported surface formats found.");
    return formats;
}

//--Returns the first supported surface color format from the preferred_formats list, or VK_FORMAT_UNDEFINED if no match found.
VkFormat CPhysicalDevice::FindSurfaceFormat(VkSurfaceKHR surface, std::vector<VkFormat> preferred_formats) {
    auto formats = SurfaceFormats(surface);  // get list of supported surface formats
    for (auto& pf : preferred_formats) 
        for (auto& f : formats) 
            if(f.format == pf) return f.format;
    return VK_FORMAT_UNDEFINED;
}

VkFormat CPhysicalDevice::FindDepthFormat(std::vector<VkFormat> preferred_formats) {
    for (auto& format : preferred_formats) {
        VkFormatProperties formatProps;
        vkGetPhysicalDeviceFormatProperties(handle, format, &formatProps);
        if (formatProps.optimalTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT) {
            return format;
        }
    }
    return VK_FORMAT_UNDEFINED; // 0
}
//----------------------------------------------------------------

const void*& getNext(void* VkStruct) {  // return end of next-chain
    struct VkStructure {
        VkStructureType sType;
        const void*     pNext;
    };
    VkStructure* p = (VkStructure*)VkStruct;
    while(p->pNext) {p = (VkStructure*)p->pNext;}
    return p->pNext;
}

//------------------------CPhysicalDevices------------------------
CPhysicalDevices::CPhysicalDevices() {}

CPhysicalDevices::CPhysicalDevices(const VkInstance instance) {
    Init(instance);
}

void CPhysicalDevices::Init(const VkInstance instance) {
    VkResult result;
    uint gpu_count = 0;
    std::vector<VkPhysicalDevice> gpus;
    do {
        result = vkEnumeratePhysicalDevices(instance, &gpu_count, NULL);  // Get number of gpu's
        if (result == VK_SUCCESS && gpu_count > 0) {
            gpus.resize(gpu_count);
            result = vkEnumeratePhysicalDevices(instance, &gpu_count, gpus.data());  // Fetch gpu list
        }
    } while (result == VK_INCOMPLETE);  // If list is incomplete, try again.
    ShowVkResult(result);
    if (!gpu_count) LOGW("No GPU devices found.");  // Vulkan driver missing?

    gpu_list.resize(gpu_count);
    for (uint i = 0; i < gpu_count; ++i) {  // for each device
        CPhysicalDevice& gpu = gpu_list[i];
        gpu.handle = gpus[i];
        vkGetPhysicalDeviceProperties(gpu, &gpu.properties);
        vkGetPhysicalDeviceFeatures  (gpu, &gpu.features);    // Vulkan 1.0

        gpu.extensions.Init(gpu);
        gpu.extensions.Add("VK_KHR_swapchain");
        gpu.extensions.Add("VK_KHR_buffer_device_address");
        //gpu.extensions.Add("VK_KHR_sampler_ycbcr_conversion");

        //--Device Features 2-- (Vulkan 1.1+)
        gpu.features2                      = {VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2};
        gpu.features_11                    = {VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_1_FEATURES};
        gpu.features_12                    = {VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES};
        //gpu.features_DescriptorIndex       = {VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_INDEXING_FEATURES};
        //gpu.features_DeviceAddress         = {VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_BUFFER_DEVICE_ADDRESS_FEATURES};
        gpu.features_AccelerationStructure = {VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ACCELERATION_STRUCTURE_FEATURES_KHR};
        gpu.features_RayTracingPipeline    = {VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PIPELINE_FEATURES_KHR};
        gpu.features_RayQuery              = {VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_QUERY_FEATURES_KHR};
        if(VK_API_VERSION == VK_API_VERSION_1_2) {
            getNext(&gpu.features2) = &gpu.features_11;
            getNext(&gpu.features2) = &gpu.features_12;
        }
        //getNext(&gpu.features2) = &gpu.features_DescriptorIndex;
        //getNext(&gpu.features2) = &gpu.features_DeviceAddress;

        bool hasRT = false;
        if(gpu.extensions.Has("VK_KHR_acceleration_structure")) hasRT=true;

        if(hasRT) {
            getNext(&gpu.features2) = &gpu.features_AccelerationStructure;
            getNext(&gpu.features2) = &gpu.features_RayTracingPipeline;
            getNext(&gpu.features2) = &gpu.features_RayQuery;
        }

        vkGetPhysicalDeviceFeatures2 (gpu, &gpu.features2);   // Vulkan 1.1+
        //---------------------

        //--Surface caps--
        // VkSurfaceCapabilitiesKHR surface_caps;
        // VKERRCHECK(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(gpu, surface, &surface_caps));
        //----------------

        // Get Queue Family properties
        uint family_count = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(gpu, &family_count, NULL);
        gpu.queue_families.resize(family_count);
        vkGetPhysicalDeviceQueueFamilyProperties(gpu, &family_count, gpu.queue_families.data());
    }
}

CPhysicalDevice* CPhysicalDevices::FindPresentable(VkSurfaceKHR surface) {
    for (auto& gpu : gpu_list)
        if (gpu.FindQueueFamily(0, surface) >= 0 ) return &gpu;
    LOGW("No devices can present to this surface. (Is DRI3 enabled?)\n");
    return nullptr;
}

void CPhysicalDevices::Print(bool show_queues) {
    printf("Physical Devices: %d\n", Count());
    for (uint i = 0; i < Count(); ++i) {  // each gpu
        CPhysicalDevice& gpu = gpu_list[i];
        VkPhysicalDeviceProperties& props = gpu.properties;
        const char* devType[]{"OTHER", "INTEGRATED", "DISCRETE", "VIRTUAL", "CPU"};
        const char* vendor = gpu.VendorName();
        printf("\t%d: %s %s %s\n", i, devType[props.deviceType], vendor, props.deviceName);
        if(show_queues){
            repeat(gpu.queue_families.size()){
                VkQueueFamilyProperties& props = gpu.queue_families[i];
                uint flags = props.queueFlags;
                printf("\t\tQueue-family:%d  count:%2d  flags:[ %s%s%s%s]\n", i, props.queueCount,
                    (flags & 1) ? "GRAPHICS " : "", (flags & 2) ? "COMPUTE " : "",
                    (flags & 4) ? "TRANSFER " : "", (flags & 8) ? "SPARSE "  : "");
            }
        }
    }
}
//----------------------------------------------------------------

//-----------------------------CDevice----------------------------
CQueue* CDevice::AddQueue(VkQueueFlags flags, VkSurfaceKHR surface) {
    ASSERT(gpu, "CDevice: No GPU selected.");
    ASSERT(!handle, "CDevice: Can't add queues after device is already in use. ");
    uint f_inx = gpu.FindQueueFamily(flags, surface);                                        // Find correct queue family
    if (f_inx < 0) { LOGW("Could not create queue with requested properties."); return 0; }  // exit if not found
    uint max = gpu.queue_families[f_inx].queueCount;                                         // max number of queues
    uint q_inx = FamilyQueueCount(f_inx);                                                    // count queues from this family
    if (q_inx == max) { LOGW("No more queues available from this family."); return 0; }      // exit if too many queues
    CQueue queue = {0, f_inx, q_inx, flags, surface, handle, gpu};                           // create queue
    queues.push_back(queue);                                                                 // add to queue list
    //Create();                                                                                // create logical device
    LOGI("Queue: %d  flags: [ %s%s%s%s]%s\n", q_inx,
         (flags & 1) ? "GRAPHICS " : "", (flags & 2) ? "COMPUTE " : "",
         (flags & 4) ? "TRANSFER " : "", (flags & 8) ? "SPARSE "  : "",
         surface ? " (can present)" : "");
    return &queues.back();
}

uint CDevice::FamilyQueueCount(uint family) {
    uint count = 0;
    for (auto& q : queues) if (q.family == family) count++;
    return count;
}

void CDevice::Create() {
    ASSERT(gpu, "CDevice: No GPU selected.");
    if (handle) Destroy();  // destroy old handle
    std::vector<float> priorities(queues.size(), 0.0f);
    std::vector<VkDeviceQueueCreateInfo> info_list;
    repeat (gpu.queue_families.size()) {
        uint queue_count = FamilyQueueCount(i);
        if (queue_count > 0) {
            VkDeviceQueueCreateInfo info = {};
            info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
            info.queueFamilyIndex = i;
            info.queueCount       = queue_count;
            info.pQueuePriorities = priorities.data();
            info_list.push_back(info);
            // LOGI("\t%d x queue_family_%d\n", queue_count, i);
        }
    }

    CDeviceExtensions& extensions = gpu.extensions;
    if(extensions.IsPicked("VK_KHR_ray_query"))              extensions.Add("VK_KHR_acceleration_structure");
    if(extensions.IsPicked("VK_KHR_ray_tracing_pipeline"))   extensions.Add("VK_KHR_acceleration_structure");
    if(extensions.IsPicked("VK_KHR_acceleration_structure")) extensions.Add("VK_KHR_deferred_host_operations");
    if(extensions.IsPicked("VK_KHR_acceleration_structure")) extensions.Add("VK_KHR_buffer_device_address");

    VkDeviceCreateInfo device_create_info = {VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO};
    device_create_info.queueCreateInfoCount    = (uint32_t)info_list.size();
    device_create_info.pQueueCreateInfos       = info_list.data();
    device_create_info.enabledExtensionCount   = extensions.PickCount();
    device_create_info.ppEnabledExtensionNames = extensions.PickList();
    device_create_info.pEnabledFeatures        = &gpu.enable_features;

    gpu.enable_features_11 = {VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_1_FEATURES};
    gpu.enable_features_12 = {VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES};
    gpu.enable_features_12.runtimeDescriptorArray = true;
    gpu.enable_features_12.shaderSampledImageArrayNonUniformIndexing=true;
    if(extensions.IsPicked("VK_EXT_descriptor_indexing"))    gpu.enable_features_12.descriptorIndexing = true;
    if(extensions.IsPicked("VK_KHR_buffer_device_address"))  gpu.enable_features_12.bufferDeviceAddress= true;
    if(extensions.IsPicked("VK_KHR_vulkan_memory_model"))    gpu.enable_features_12.vulkanMemoryModel  = true;
    if(extensions.IsPicked("VK_KHR_acceleration_structure")) gpu.enable_features_12.hostQueryReset     = true; // for compaction

    if(extensions.IsPicked("VK_KHR_sampler_ycbcr_conversion"))gpu.enable_features_11.samplerYcbcrConversion=true;

    if(extensions.IsPicked("VK_KHR_acceleration_structure")) gpu.enable_features_AccelerationStructure = gpu.features_AccelerationStructure;
    if(extensions.IsPicked("VK_KHR_ray_tracing_pipeline"))   gpu.enable_features_RayTracingPipeline    = gpu.features_RayTracingPipeline;
    if(extensions.IsPicked("VK_KHR_ray_query"))              gpu.enable_features_RayQuery              = gpu.features_RayQuery;

    gpu.enable_features_AccelerationStructure.accelerationStructureCaptureReplay = false;
    //gpu.enable_features_AccelerationStructure.accelerationStructureHostCommands  = true;  // not supported on 1080
    gpu.enable_features_AccelerationStructure.pNext = 0;
    gpu.enable_features_RayQuery.pNext              = 0;
    gpu.enable_features_RayTracingPipeline.pNext    = 0;
    gpu.enable_features.depthClamp = true;                // clamp skybox to farplane

    if(VK_API_VERSION == VK_API_VERSION_1_2) {
        getNext(&device_create_info) = &gpu.enable_features_11;
        getNext(&device_create_info) = &gpu.enable_features_12;
    }

    if(extensions.IsPicked("VK_KHR_acceleration_structure")) getNext(&device_create_info) = &gpu.enable_features_AccelerationStructure;
    if(extensions.IsPicked("VK_KHR_ray_tracing_pipeline"))   getNext(&device_create_info) = &gpu.enable_features_RayTracingPipeline;
    if(extensions.IsPicked("VK_KHR_ray_query"))              getNext(&device_create_info) = &gpu.enable_features_RayQuery;


/*  // PRE-VULKAN 1.2 METHOD: (deprecated.  Use: "enable_features_12" instead.)
    VkPhysicalDeviceDescriptorIndexingFeatures  enable_features_DescriptorIndex = {};
    VkPhysicalDeviceBufferDeviceAddressFeatures enable_features_DeviceAddress = {};

    // Enables descriptor indexing, if VK_EXT_descriptor_indexing is picked
    //-----------------------------------------------------------------------------------
    if(extensions.IsPicked("VK_EXT_descriptor_indexing")) {
        enable_features_DescriptorIndex = gpu.features_DescriptorIndex;
        enable_features_DescriptorIndex.pNext = 0;
        getNext(&device_create_info) = &enable_features_DescriptorIndex;
    }
    //-----------------------------------------------------------------------------------

    // Enables Buffer Device Address features, if VK_KHR_buffer_device_address is picked
    //-----------------------------------------------------------------------------------
    if(extensions.IsPicked("VK_KHR_buffer_device_address")) {
        enable_features_DeviceAddress = gpu.features_DeviceAddress;
        enable_features_DeviceAddress.pNext = 0;
        getNext(&device_create_info) = &enable_features_DeviceAddress;
    }
    //-----------------------------------------------------------------------------------
*/


/*
    // Enables descriptor indexing features, if VK_EXT_descriptor_indexing is picked
    //-------------------------------------------------------------------------------
    if(extensions.IsPicked("VK_EXT_descriptor_indexing")) {
        VkPhysicalDeviceDescriptorIndexingFeaturesEXT descIndexFeatures = {};
        descIndexFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_INDEXING_FEATURES;
        VkPhysicalDeviceFeatures2 features2 = {};
        supportedFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
        supportedFeatures.pNext = &descIndexFeatures;
        vkGetPhysicalDeviceFeatures2(gpu, &supportedFeatures);
        getNext(&device_create_info) = &descIndexFeatures;
        //device_create_info.pNext     = &descIndexFeatures;
    }
    //-------------------------------------------------------------------------------
*/

    VKERRCHECK(vkCreateDevice(gpu, &device_create_info, nullptr, &handle));         // create device
    //for (auto& q : queues) vkGetDeviceQueue(handle, q.family, q.index, &q.handle);  // get queue handles
    for (auto& q : queues) {
        q.device = handle;
        vkGetDeviceQueue(handle, q.family, q.index, &q.queue);  // get queue handles
    }
}

void CDevice::Destroy(){
    if (!handle) return;
    vkDeviceWaitIdle(handle);
    vkDestroyDevice(handle, nullptr);
    handle = nullptr;
}

CDevice::CDevice(CPhysicalDevice gpu) : handle() {
    SelectGPU(gpu);
}

void CDevice::SelectGPU(CPhysicalDevice gpu) {
    ASSERT(!this->gpu, "GPU already selected.");
    this->gpu = gpu;
    queues.reserve(16);
    LOGI("Logical Device using GPU: %s (Vulkan %s)\n",gpu.properties.deviceName, gpu.APIVersion());
#ifdef ENABLE_VALIDATION
    gpu.extensions.Print();
#endif
}

CDevice::~CDevice() {
    Destroy();
    LOGI("Logical device destroyed\n");
}

//------------------------------------CQueue---------------------------------------
VkCommandPool CQueue::CreateCommandPool() const {
    VkCommandPoolCreateInfo poolInfo = {};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.queueFamilyIndex = family;
    poolInfo.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT | VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    VkCommandPool command_pool;
    VKERRCHECK(vkCreateCommandPool(device, &poolInfo, nullptr, &command_pool));
    return command_pool;
}

VkCommandBuffer CQueue::CreateCommandBuffer(VkCommandPool command_pool) const {
    VkCommandBufferAllocateInfo allocInfo = {VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO};
    allocInfo.commandPool = command_pool;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = 1;
    VkCommandBuffer command_buffer;
    VKERRCHECK(vkAllocateCommandBuffers(device, &allocInfo, &command_buffer));
    return command_buffer;
}
//---------------------------------------------------------------------------------

/*
void CDevice::Print() {  // List created queues
    printf("Logical Device queues:\n");
    uint qcount = queues.size();
    repeat (qcount){
        CQueue& q = queues[i];
        printf("\t%d: family=%d index=%d presentable=%s flags=", i, q.family, q.index, q.presentable ? "True" : "False");
        const char* fnames[]{"GRAPHICS", "COMPUTE", "TRANSFER", "SPARSE"};
        repeat(4) if ((q.flags & 1 << i)) printf("%s ", fnames[i]);
        printf("\n");
    }
}
*/

//------------------------------CCmd------------------------------
CCmd::CCmd() : device(), queue(), command_pool(), command_buffer(), fence() {}

CCmd::CCmd(const CQueue& cqueue) {
    SelectQueue(cqueue);
}

void CCmd::SelectQueue(const CQueue& cqueue) {
    device         = cqueue.device;
    queue          = cqueue.queue;
    ASSERT(!!queue, "queue not yet initialized\n");
    command_pool   = cqueue.CreateCommandPool();
    command_buffer = cqueue.CreateCommandBuffer(command_pool);
    //---Fence---
    VkFenceCreateInfo createInfo = {VK_STRUCTURE_TYPE_FENCE_CREATE_INFO};
    createInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
    vkCreateFence(device, &createInfo, nullptr, &fence);
    //-----------
}

CCmd::~CCmd() {
    if(fence)          vkDestroyFence(device, fence, nullptr);
    if(command_buffer) vkFreeCommandBuffers(device, command_pool, 1, &command_buffer);
    if(command_pool)   vkDestroyCommandPool(device, command_pool, nullptr);
}

void CCmd::Begin(VkCommandBufferUsageFlags flags) {
    ASSERT(queue, "CCmd: No Queue selected.\n");
    ASSERT(recording==false, "Command buffer is already recording.\n");
    vkWaitForFences(device, 1, &fence, VK_TRUE, UINT64_MAX);
    VkCommandBufferBeginInfo cmdBufBeginInfo = { VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO };
    cmdBufBeginInfo.flags = flags;  // = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT
    VKERRCHECK( vkBeginCommandBuffer(command_buffer, &cmdBufBeginInfo) );
    recording = true;
}

void CCmd::End(bool submit) {
    ASSERT(recording==true, "Command buffer is not recording.\n");
    VKERRCHECK(vkEndCommandBuffer(command_buffer));
    if(submit) Submit();
    recording = false;
}

void CCmd::Submit(bool wait) {
    VkSubmitInfo submitInfo = { VK_STRUCTURE_TYPE_SUBMIT_INFO };
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &command_buffer;
    //VKERRCHECK(vkQueueSubmit(queue, 1, &submitInfo, VK_NULL_HANDLE))
    vkResetFences(device, 1, &fence);
    VKERRCHECK(vkQueueSubmit(queue, 1, &submitInfo, fence))
    if(wait) Wait();
}

void CCmd::Wait() {
    //VKERRCHECK(vkQueueWaitIdle(queue));
    VKERRCHECK(vkWaitForFences(device, 1, &fence, VK_TRUE, UINT64_MAX));
}
//----------------------------------------------------------------
