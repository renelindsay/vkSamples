#include "Buffers.h"
#include "VkFormats.h"

//#define NOMINMAX
//#define VMA_RECORDING_ENABLED      0
//#define VMA_DEDICATED_ALLOCATION   0
//#define VMA_STATS_STRING_ENABLED   1
#define VMA_STATIC_VULKAN_FUNCTIONS  0  // NOT statically linked to Vulkan
#define VMA_DYNAMIC_VULKAN_FUNCTIONS 1  // Fetch Vulkan functions automatically

#define VMA_IMPLEMENTATION
#include "vk_mem_alloc.h"
//#include <math.h>

#define GetProc(PROC) PROC = (PFN_##PROC) vkGetDeviceProcAddr(device, #PROC);  // Todo: remove

#define MEMORY_BUDGET  // Todo: remove

CAllocator* default_allocator = nullptr;

uint32_t RoundUp(uint32_t val, uint32_t multiple){ return ((val - 1) / multiple + 1) * multiple; }

static VkExtent2D Extent2D(const CImageBase& img) { return {img.Width(), img.Height()}; }
static VkExtent3D Extent3D(const VkExtent2D ext2D){ return {ext2D.width, ext2D.height, 1}; }

void setLayout(VkCommandBuffer cmd, VkImage image, VkImageLayout oldLayout, VkImageLayout newLayout, uint32_t baseMipLevel, uint32_t mipLevels, uint32_t layers) {
    ASSERT(newLayout != VK_IMAGE_LAYOUT_UNDEFINED, "Can't set layout to VK_IMAGE_LAYOUT_UNDEFINED.");
    if(oldLayout == newLayout) return;
    VkImageMemoryBarrier barrier = {VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER};
    barrier.oldLayout = oldLayout;
    barrier.newLayout = newLayout;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.image = image;
    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;  // NOTE: must match value in ImageView
    barrier.subresourceRange.baseMipLevel   = baseMipLevel; //0;
    barrier.subresourceRange.levelCount     = mipLevels;    //1;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount     = layers;       //1;

    if(newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
        barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;

    struct LayoutSet { uint32_t AccessMask; uint32_t Stage; };  //what,where
    static const std::map<VkImageLayout, LayoutSet> layouts = {
        {VK_IMAGE_LAYOUT_UNDEFINED,                        {0,                                            VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT            }},  //0
        {VK_IMAGE_LAYOUT_GENERAL,                          {0,                                            VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT         }},  //1
        {VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,         {VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,         VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT}},  //2
        {VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, {VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT |
                                                            VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT, VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT   }},  //3
        {VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL,  {VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT,  VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT        }},  //4
        {VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,         {VK_ACCESS_SHADER_READ_BIT,                    VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT        }},  //5
        {VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,             {VK_ACCESS_TRANSFER_READ_BIT,                  VK_PIPELINE_STAGE_TRANSFER_BIT               }},  //6
        {VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,             {VK_ACCESS_TRANSFER_WRITE_BIT,                 VK_PIPELINE_STAGE_TRANSFER_BIT               }},  //7
        {VK_IMAGE_LAYOUT_PREINITIALIZED,                   {0,                                            VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT            }},  //8  (Linear images only)
        {VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,                  {0,                                            VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT         }},

    };
    auto prev_it = layouts.find(oldLayout);  if(prev_it == layouts.end()) { LOGE("Unsupported image layout (oldLayout)\n"); return;}
    auto next_it = layouts.find(newLayout);  if(next_it == layouts.end()) { LOGE("Unsupported image layout (newLayout)\n"); return;}
    LayoutSet prev = prev_it->second;
    LayoutSet next = next_it->second;
    barrier.srcAccessMask = prev.AccessMask;
    barrier.dstAccessMask = next.AccessMask;

    VkDependencyFlags depFlags = VK_DEPENDENCY_BY_REGION_BIT;
    vkCmdPipelineBarrier(cmd,
        prev.Stage,  // We wait for prev-stage to finish
        next.Stage,  // Next-stage waits for us to finish
        depFlags,
        0, nullptr,  // MemoryBarriers
        0, nullptr,  // BufferMemoryBarriers
        1, &barrier  // ImageMemoryBarriers
    );
}
//------------------------------------------------------------------------------------------------

//---------------------ALLOCATOR---------------------
CAllocator::CAllocator() : allocator() {}

CAllocator::CAllocator(VkInstance instance, const CQueue& queue, VkDeviceSize blockSize) : allocator(0) {
    Init(instance, queue, blockSize);
}

void CAllocator::Init(VkInstance instance, const CQueue& queue, VkDeviceSize blockSize) { // Select a transfer queue for staging operations.
    this->instance = instance;
    CCmd::SelectQueue(queue);
    gpu            = queue.gpu;
    device         = queue.device;
    //this->queue    = queue;
    //command_pool   = queue.CreateCommandPool();
    //command_buffer = queue.CreateCommandBuffer(command_pool);

    VmaAllocatorCreateInfo allocatorInfo = {};
    allocatorInfo.physicalDevice = gpu;
    allocatorInfo.device = device;
    allocatorInfo.instance = instance;
    allocatorInfo.preferredLargeHeapBlockSize = blockSize;
#ifdef MEMORY_BUDGET
    allocatorInfo.vulkanApiVersion = VK_API_VERSION;  // VK_API_VERSION_1_2;
    allocatorInfo.flags = VMA_ALLOCATOR_CREATE_EXT_MEMORY_BUDGET_BIT;         // Enables usage of VK_EXT_memory_budget extension.
#endif
    if(VK_API_VERSION >= VK_API_VERSION_1_2) {
        allocatorInfo.flags |= VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT;    // Enables usage of VK_KHR_buffer_device_address
    }
#if (VMA_DYNAMIC_VULKAN_FUNCTIONS == 0)
    VmaVulkanFunctions fn {};
    fn.vkAllocateMemory                    = (PFN_vkAllocateMemory)vkAllocateMemory;
    fn.vkBindBufferMemory                  = (PFN_vkBindBufferMemory)vkBindBufferMemory;
    fn.vkBindImageMemory                   = (PFN_vkBindImageMemory)vkBindImageMemory;
    fn.vkCmdCopyBuffer                     = (PFN_vkCmdCopyBuffer)vkCmdCopyBuffer;
    fn.vkCreateBuffer                      = (PFN_vkCreateBuffer)vkCreateBuffer;
    fn.vkCreateImage                       = (PFN_vkCreateImage)vkCreateImage;
    fn.vkDestroyBuffer                     = (PFN_vkDestroyBuffer)vkDestroyBuffer;
    fn.vkDestroyImage                      = (PFN_vkDestroyImage)vkDestroyImage;
    fn.vkFlushMappedMemoryRanges           = (PFN_vkFlushMappedMemoryRanges)vkFlushMappedMemoryRanges;
    fn.vkFreeMemory                        = (PFN_vkFreeMemory)vkFreeMemory;
    fn.vkGetBufferMemoryRequirements       = (PFN_vkGetBufferMemoryRequirements)vkGetBufferMemoryRequirements;
    fn.vkGetImageMemoryRequirements        = (PFN_vkGetImageMemoryRequirements)vkGetImageMemoryRequirements;
    fn.vkGetPhysicalDeviceMemoryProperties = (PFN_vkGetPhysicalDeviceMemoryProperties)vkGetPhysicalDeviceMemoryProperties;
    fn.vkGetPhysicalDeviceProperties       = (PFN_vkGetPhysicalDeviceProperties)vkGetPhysicalDeviceProperties;
    fn.vkInvalidateMappedMemoryRanges      = (PFN_vkInvalidateMappedMemoryRanges)vkInvalidateMappedMemoryRanges;
    fn.vkMapMemory                         = (PFN_vkMapMemory)vkMapMemory;
    fn.vkUnmapMemory                       = (PFN_vkUnmapMemory)vkUnmapMemory;
    fn.vkGetBufferMemoryRequirements       = (PFN_vkGetBufferMemoryRequirements)vkGetBufferMemoryRequirements;
    fn.vkGetImageMemoryRequirements        = (PFN_vkGetImageMemoryRequirements)vkGetImageMemoryRequirements;

  #ifdef MEMORY_BUDGET
    fn.vkGetBufferMemoryRequirements2KHR   = (PFN_vkGetBufferMemoryRequirements2KHR)vkGetBufferMemoryRequirements2KHR;                // requires: VK_KHR_get_memory_requirements2         (device extension)
    fn.vkGetImageMemoryRequirements2KHR    = (PFN_vkGetImageMemoryRequirements2KHR)vkGetImageMemoryRequirements2KHR;                  // requires: VK_KHR_get_memory_requirements2         (device extension)
    fn.vkBindBufferMemory2KHR              = (PFN_vkBindBufferMemory2KHR)vkBindBufferMemory2KHR;                                      // requires: VK_KHR_bind_memory2                     (device extension)
    fn.vkBindImageMemory2KHR               = (PFN_vkBindImageMemory2KHR)vkBindImageMemory2KHR;                                        // requires: VK_KHR_bind_memory2                     (device extension)
    fn.vkGetPhysicalDeviceMemoryProperties2KHR=(PFN_vkGetPhysicalDeviceMemoryProperties2KHR)vkGetPhysicalDeviceMemoryProperties2KHR;  // requires: VK_KHR_get_physical_device_properties2  (instance extension)
    if(fn.vkBindImageMemory2KHR==0) {
      fn.vkGetBufferMemoryRequirements2KHR = (PFN_vkGetBufferMemoryRequirements2KHR)vkGetBufferMemoryRequirements2;                  // requires: Vulkan 1.1
      fn.vkGetImageMemoryRequirements2KHR  = (PFN_vkGetImageMemoryRequirements2KHR)vkGetImageMemoryRequirements2;                    // requires: Vulkan 1.1
      fn.vkBindBufferMemory2KHR            = (PFN_vkBindBufferMemory2KHR)vkBindBufferMemory2;                                        // requires: Vulkan 1.1
      fn.vkBindImageMemory2KHR             = (PFN_vkBindImageMemory2KHR)vkBindImageMemory2;                                          // requires: Vulkan 1.1
      fn.vkGetPhysicalDeviceMemoryProperties2KHR=(PFN_vkGetPhysicalDeviceMemoryProperties2KHR)vkGetPhysicalDeviceMemoryProperties2;  // requires: Vulkan 1.1
    }
  #endif  // MEMORY_BUDGET

    allocatorInfo.pVulkanFunctions = &fn;
#elif (VMA_DYNAMIC_VULKAN_FUNCTIONS == 1)
    VmaVulkanFunctions fn {};
    fn.vkGetInstanceProcAddr = vkGetInstanceProcAddr;
    fn.vkGetDeviceProcAddr = vkGetDeviceProcAddr;
    allocatorInfo.pVulkanFunctions = &fn;
#endif  // VMA_DYNAMIC_VULKAN_FUNCTIONS

    vmaCreateAllocator(&allocatorInfo, &allocator);
    if(!default_allocator) default_allocator = this;
    LOGI("VMA Allocator created\n");

    yuv_sampler.Create(device);
}

CAllocator::~CAllocator() {
    if(default_allocator == this) {
        default_allocator = 0;
    }
    vmaDestroyAllocator(allocator);

    //if(command_buffer)  vkFreeCommandBuffers(device, command_pool, 1, &command_buffer);
    //if(command_pool)    vkDestroyCommandPool(device, command_pool, nullptr);
    //if(allocator)       vmaDestroyAllocator(allocator);
    //if(default_allocator == this) default_allocator = 0;
    LOGI("VMA Allocator destroyed\n");
}

//--------------------------------vmaBuffer-------------------------------
vmaBuffer CAllocator::vkmalloc(uint64_t size, VkBufferUsageFlags usage, bool mapped) {
    VkBufferCreateInfo bufInfo = {VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO};
    bufInfo.size = size;
    bufInfo.usage = usage;  //VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
    bufInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    VmaAllocationCreateInfo allocCreateInfo = {};
    allocCreateInfo.usage = mapped ? VMA_MEMORY_USAGE_CPU_ONLY : VMA_MEMORY_USAGE_GPU_ONLY;
    allocCreateInfo.flags = mapped ? VMA_ALLOCATION_CREATE_MAPPED_BIT : 0;

    vmaBuffer sb{};
    ASSERT(!!&vmaCreateBuffer, "Allocator was not initialized\n");
    VKERRCHECK(vmaCreateBuffer(allocator, &bufInfo, &allocCreateInfo,
                               &sb.buffer, &sb.bufferAlloc, &sb.allocInfo));
    return sb;
}

void CAllocator::vkfree(vmaBuffer vbuf) {
    vmaDestroyBuffer(allocator, vbuf.buffer, vbuf.bufferAlloc);
}
//------------------------------------------------------------------------

void CAllocator::CreateBuffer(const void* data, uint64_t size, VkFlags usage, VmaMemoryUsage memtype, VkBuffer& buffer, VmaAllocation& alloc, void** mapped) {
    if(useRTX) usage |= VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT | VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR;

    vmaBuffer buf;
    if(memtype != VMA_MEMORY_USAGE_GPU_ONLY) {
        buf = vkmalloc(size, usage, true);
        if(data) { memcpy(buf, data, size); } else { memset(buf, 0, size); }
        if(mapped) { *mapped = buf; }
    }else{
        // For GPU-only memory, copy via staging buffer.  // TODO: Also skip staging buffer on integrated GPUs.
        vmaBuffer stage_buf = vkmalloc(size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT);                 // staging buffer
                        buf = vkmalloc(size, VK_BUFFER_USAGE_TRANSFER_DST_BIT | usage, false);  // GPU buffer
        if(data) { memcpy(stage_buf, data, size); } else { memset(stage_buf, 0, size); }        //copy to staging buffer
        BeginCmd();                                                                             //copy to GPU buffer
            VkBufferCopy bufCopyRegion = {};
            bufCopyRegion.srcOffset = 0;
            bufCopyRegion.dstOffset = 0;
            bufCopyRegion.size = size;
            vkCmdCopyBuffer(command_buffer, stage_buf, buf, 1, &bufCopyRegion);
        EndCmd();
        vkfree(stage_buf);
    }
    buffer = buf.buffer;
    alloc  = buf.bufferAlloc;
    buf_stats += alloc->GetSize();
}

void CAllocator::DestroyBuffer(VkBuffer buffer, VmaAllocation alloc) {
    buf_stats -= alloc->GetSize();
    vmaDestroyBuffer(allocator, buffer, alloc);
}
//---------------------------------------------------
//Final layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
void CAllocator::CreateImage(const void* data, VkExtent3D extent, VkFormat format, uint32_t mipLevels,
                             VkImage& image, VmaAllocation& alloc, VkImageView& view) {
    VkImageUsageFlags usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT |
                              VK_IMAGE_USAGE_TRANSFER_DST_BIT |
                              VK_IMAGE_USAGE_SAMPLED_BIT;
    CreateImage(data, extent, format, mipLevels, 1, VK_IMAGE_VIEW_TYPE_2D, usage, image, alloc, view);
}

//Final layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
void CAllocator::CreateImage(const void* data, VkExtent3D extent, VkFormat format, uint32_t mipLevels, uint32_t arrayLayers, VkImageViewType viewType, VkImageUsageFlags usage,
                             VkImage& image, VmaAllocation& alloc, VkImageView& view, void** mapped) {
    //if(mipmap) mipLevels = (uint32_t)(Log2(std::max(extent.width, extent.height))) + 1;

    // Create Image in GPU memory
    VkImageCreateInfo imageInfo = {VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO};
    imageInfo.flags       = 0;
    //imageInfo.flags = VK_IMAGE_CREATE_MUTABLE_FORMAT_BIT;
    if(viewType==VK_IMAGE_VIEW_TYPE_CUBE) imageInfo.flags = VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;
    imageInfo.imageType   = VK_IMAGE_TYPE_2D;
    imageInfo.format      = format; //VK_FORMAT_R8G8B8A8_SRGB;
    imageInfo.extent      = extent;
    imageInfo.mipLevels   = mipLevels; //1;
    imageInfo.arrayLayers = arrayLayers;
    imageInfo.samples     = VK_SAMPLE_COUNT_1_BIT;
    imageInfo.tiling      = mapped ? VK_IMAGE_TILING_LINEAR : VK_IMAGE_TILING_OPTIMAL;
    imageInfo.usage       = usage;  //VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
    imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    //imageInfo.queueFamilyIndexCount = 0;
    //imageInfo.pQueueFamilyIndices   = nullptr;
    imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

    VmaAllocationCreateInfo allocCreateInfo2 = {};
    //allocCreateInfo2.usage = VMA_MEMORY_USAGE_GPU_ONLY;  //memtype
    allocCreateInfo2.usage = mapped? VMA_MEMORY_USAGE_CPU_TO_GPU : VMA_MEMORY_USAGE_GPU_ONLY;  //memtype
    allocCreateInfo2.flags = mapped? VMA_ALLOCATION_CREATE_MAPPED_BIT : 0;
    vmaCreateImage(allocator, &imageInfo, &allocCreateInfo2, &image, &alloc, nullptr);

    // Create ImageView
    auto fmt_info = FormatInfo(format);
    VkImageViewCreateInfo imageViewInfo = { VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO };
    //imageViewInfo.pNext = (format==VK_FORMAT_G8B8G8R8_422_UNORM) ? &yuv_sampler.info : 0;  // YUV
    imageViewInfo.pNext = (fmt_info.isYUV()) ? &yuv_sampler.info : 0;  // YUV
    imageViewInfo.image    = image;
    imageViewInfo.viewType = viewType;  // VK_IMAGE_VIEW_TYPE_2D or VK_IMAGE_VIEW_TYPE_CUBE
    imageViewInfo.format   = format;  //VK_FORMAT_R8G8B8A8_UNORM;

    if(fmt_info.channels == 1) {  // Grayscale
        imageViewInfo.components.r = VK_COMPONENT_SWIZZLE_R;
        imageViewInfo.components.g = VK_COMPONENT_SWIZZLE_R;
        imageViewInfo.components.b = VK_COMPONENT_SWIZZLE_R;
        imageViewInfo.components.a = VK_COMPONENT_SWIZZLE_A;
    }
    imageViewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    imageViewInfo.subresourceRange.baseMipLevel   = 0;
    imageViewInfo.subresourceRange.levelCount     = mipLevels; //1;
    imageViewInfo.subresourceRange.baseArrayLayer = 0;
    imageViewInfo.subresourceRange.layerCount     = arrayLayers;
    VKERRCHECK( vkCreateImageView(device, &imageViewInfo, nullptr, &view) );

    // Copy data to image, using staging buffer
    VkImageLayout layout = VK_IMAGE_LAYOUT_UNDEFINED;
    WriteImage(image, layout, extent, format, data, 0, mipLevels, arrayLayers);
    img_stats += alloc->GetSize();

    if (mapped) vmaMapMemory(allocator, alloc, mapped);
}

// For Swapchain Depth/Color attachments
// Final layout = VK_IMAGE_LAYOUT_UNDEFINED
void CAllocator::CreateImage(VkExtent2D extent, VkFormat format, VkSampleCountFlagBits samples, VkImageUsageFlags usage, VkImage& image, VmaAllocation& alloc, VkImageView& view) {
    // Create Image
    VkImageCreateInfo imageInfo = {VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO};
    imageInfo.imageType   = VK_IMAGE_TYPE_2D;
    imageInfo.format      = format;  //VK_FORMAT_D32_SFLOAT
    imageInfo.extent      = Extent3D(extent);
    imageInfo.mipLevels   = 1;
    imageInfo.arrayLayers = 1;
    imageInfo.samples     = samples; //VK_SAMPLE_COUNT_1_BIT;
    imageInfo.tiling      = VK_IMAGE_TILING_OPTIMAL;
    imageInfo.usage       = usage;   //VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT
    imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    //VKERRCHECK(vkCreateImage(device, &imageInfo, nullptr, &image));  // NoVMA

    // VMA
    VmaAllocationCreateInfo allocInfo = {};
    allocInfo.flags = VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT;

    if(usage & VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT) {  // For Transient images, try Lazy_alloc (mobile tile-based)
        allocInfo.usage = VMA_MEMORY_USAGE_GPU_LAZILY_ALLOCATED;
        vmaCreateImage(allocator, &imageInfo, &allocInfo, &image, &alloc, nullptr);
    }
    if(!image) {                                           // If Lazy fails (Desktop), use GPU memory
        allocInfo.usage = VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE;
        vmaCreateImage(allocator, &imageInfo, &allocInfo, &image, &alloc, nullptr);
    }

    VkImageAspectFlags aspectFlags=0;
    if(usage & VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT)         aspectFlags = VK_IMAGE_ASPECT_COLOR_BIT;
    if(usage & VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT) aspectFlags = VK_IMAGE_ASPECT_DEPTH_BIT;// | VK_IMAGE_ASPECT_STENCIL_BIT;

    // Create ImageView
    VkImageViewCreateInfo viewInfo = {VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO};
    viewInfo.image    = image;
    viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    viewInfo.format   = format;
    //viewInfo.components.r = VK_COMPONENT_SWIZZLE_R;
    //viewInfo.components.g = VK_COMPONENT_SWIZZLE_G;
    //viewInfo.components.b = VK_COMPONENT_SWIZZLE_B;
    //viewInfo.components.a = VK_COMPONENT_SWIZZLE_A;
    viewInfo.subresourceRange.aspectMask     = aspectFlags;
    viewInfo.subresourceRange.baseMipLevel   = 0;
    viewInfo.subresourceRange.levelCount     = 1;  // no mipmaps
    viewInfo.subresourceRange.baseArrayLayer = 0;
    viewInfo.subresourceRange.layerCount     = 1;
    VKERRCHECK(vkCreateImageView(device, &viewInfo, nullptr, &view));

    img_stats += alloc->GetSize();
}
//------------------------------------------------------------------------

//--------------------------------WriteImage------------------------------
// TODO: Use Transfer queue
// Final layout : VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
void CAllocator::WriteImage(VkImage& image, VkImageLayout layout, VkExtent3D extent, VkFormat format, const void* data,
                            uint32_t mipLevel, uint32_t mipLevels, uint32_t arrayLayers) {
    format_info fmt = FormatInfo(format);
    extent.width >>=mipLevel;
    extent.height>>=mipLevel;
    uint64_t size = extent.width * extent.height * extent.depth * fmt.size * arrayLayers;
    if(fmt.isCompressed()) size /= 16;

    auto stagebuf = vkmalloc(size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT);
    if(data) memcpy(stagebuf, data, size);  // data may be nullptr
    else     memset(stagebuf,    0, size);  // if not data, clear to black

    //  Copy image from staging buffer to texture
    BeginCmd();
        SetImageLayout(image, layout, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, mipLevel, mipLevels, arrayLayers);
        VkBufferImageCopy region = {};
        region.bufferOffset     =0;
        region.bufferRowLength  =0;
        region.bufferImageHeight=0;
        region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        region.imageSubresource.mipLevel       = mipLevel;
        region.imageSubresource.baseArrayLayer = 0;
        region.imageSubresource.layerCount     = arrayLayers;
        region.imageOffset = {0, 0, 0};
        region.imageExtent = extent;
        vkCmdCopyBufferToImage(command_buffer, stagebuf, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);
        SetImageLayout(image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, mipLevel, mipLevels, arrayLayers);
    EndCmd();
    vkfree(stagebuf);

    if(mipLevels > 1) GenerateMipmaps(image, format, extent.width, extent.height, mipLevels, arrayLayers);
}
//------------------------------------------------------------------------
//--------------------------------ReadImage-------------------------------
// TODO: use vkCmdBlitImage for automatic BGRA to RGBA conversion
// TODO: miplevels
// TODO: Use Transfer queue
void CAllocator::ReadImage(VkImage& image, VkImageLayout layout, VkExtent3D extent, VkFormat format, void* data) {
    uint32_t fmt_size = FormatInfo(format).size;
    uint32_t mipLevels   = 1;
    uint32_t arrayLayers = 1;

    // Copy image data to staging buffer in CPU memory
    uint64_t size = extent.width * extent.height * extent.depth * fmt_size;
    auto stagebuf = vkmalloc(size, VK_BUFFER_USAGE_TRANSFER_DST_BIT);

    //  Copy image from texture to staging buffer
    BeginCmd();
        SetImageLayout(image, layout, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, 0, mipLevels, arrayLayers);
        VkBufferImageCopy region = {};
        region.bufferOffset     =0;
        region.bufferRowLength  =0;
        region.bufferImageHeight=0;
        region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        region.imageSubresource.mipLevel       = 0;
        region.imageSubresource.baseArrayLayer = 0;
        region.imageSubresource.layerCount     = arrayLayers;
        region.imageOffset = {0, 0, 0};
        region.imageExtent = extent;
        vkCmdCopyImageToBuffer(command_buffer, image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, stagebuf, 1, &region);
        SetImageLayout(image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, layout, 0, mipLevels, arrayLayers);
    EndCmd();

    memcpy(data, stagebuf, size);
    vkfree(stagebuf);
}
//------------------------------------------------------------------------

void CAllocator::SetImageLayout(VkImage image, VkImageLayout oldLayout, VkImageLayout newLayout, uint32_t baseMipLevel, uint32_t mipLevels, uint32_t layers) {
    if(oldLayout == newLayout) return;
    bool isRecording = recording;
    if(!isRecording) BeginCmd();
    setLayout(command_buffer, image, oldLayout, newLayout, baseMipLevel, mipLevels, layers);
    if(!isRecording) EndCmd();
}

//---------------------------------Mipmaps--------------------------------
bool CAllocator::GenerateMipmaps(VkImage image, VkFormat imageFormat, int32_t texWidth, int32_t texHeight, uint32_t mipLevels, uint32_t arrayLayers) {
    // Check if image format supports linear blitting
    VkFormatProperties formatProperties;
    vkGetPhysicalDeviceFormatProperties(gpu, imageFormat, &formatProperties);

    if (!(formatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT)) {
        LOGW("GenerateMipmaps: Texture image format does not support linear blitting.\n");
        return false;
    }

    if (!(formatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_BLIT_DST_BIT)) {
        LOGW("GenerateMipmaps: Texture image format does not support blitting.\n");
        return false;
    }

    LOGI("Generate mipmaps on GPU(%d)\n", mipLevels);
    BeginCmd();
    int32_t mipW = texWidth;
    int32_t mipH = texHeight;

    SetImageLayout(image, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 0, mipLevels, arrayLayers);
    for (uint32_t i = 1; i < mipLevels; i++) {  // largest image : i=0
        uint32_t baseMip = i-1;
        SetImageLayout(image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, baseMip, 1, arrayLayers);

        VkImageBlit blit = {};
        blit.srcOffsets[0] = {0, 0, 0};
        blit.srcOffsets[1] = {mipW, mipH, 1};
        blit.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        blit.srcSubresource.mipLevel = i - 1;
        blit.srcSubresource.baseArrayLayer = 0;
        blit.srcSubresource.layerCount = arrayLayers;
        blit.dstOffsets[0] = {0, 0, 0};
        if(mipW > 1) mipW /= 2;
        if(mipH > 1) mipH /= 2;
        blit.dstOffsets[1] = { mipW, mipH, 1 };
        blit.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        blit.dstSubresource.mipLevel = i;
        blit.dstSubresource.baseArrayLayer = 0;
        blit.dstSubresource.layerCount = arrayLayers;

        vkCmdBlitImage(command_buffer,
            image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
            image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            1, &blit,
            VK_FILTER_LINEAR);

        SetImageLayout(image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, baseMip, 1, arrayLayers);
    }
    SetImageLayout(image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, mipLevels-1, 1, arrayLayers);
    EndCmd();
    return true;
}
//------------------------------------------------------------------------
//---------------------------------------------------
void CAllocator::DestroyImage(VkImage image, VkImageView view, VmaAllocation alloc) {
    if(view) vkDestroyImageView(device, view, nullptr);
    if(image) vmaDestroyImage(allocator, image, alloc);
    img_stats -= alloc->GetSize();
}
//---------------------------------------------------
//---------------------GetBudget---------------------
std::vector<VmaBudget> CAllocator::GetBudget() {
    uint32_t cnt = allocator->GetMemoryHeapCount();
    std::vector<VmaBudget> heaps(cnt);
    vmaGetHeapBudgets(allocator, &heaps[0]);
    return heaps;
}
//---------------------------------------------------
//---------------------CvkBuffer---------------------
CvkBuffer::CvkBuffer(CAllocator& allocator) : allocator(&allocator), allocation(), buffer(), count(), stride() {}
CvkBuffer::CvkBuffer() : allocator(default_allocator), allocation(), buffer(), count(), stride() {}
CvkBuffer::~CvkBuffer() { Clear(); }

void CvkBuffer::Clear() {
    if(!allocator) { allocator = default_allocator; }
    if(buffer) {
        VKERRCHECK(vkQueueWaitIdle(allocator->queue));
        allocator->DestroyBuffer(buffer, allocation);
    }
    buffer = 0;
    count  = 0;
    stride = 0;
    mapped = 0;
}

// move construction
void CvkBuffer::swap(CvkBuffer& other) {
    std::swap(allocator, other.allocator);
    std::swap(allocation,other.allocation);
    std::swap(buffer,    other.buffer);
    std::swap(count,     other.count);
    std::swap(stride,    other.stride);
    std::swap(mapped,    other.mapped);
}

void CvkBuffer::Data(const void* data, uint32_t count, uint32_t stride, VkFlags usage, VmaMemoryUsage memtype, void** mapped ) {
    if(mapped&&(this->count==count)&&(this->stride==stride)) return;  // reuse existing buffer   
    Clear();
    ASSERT(!!allocator, "VMA Allocator not initialized.");
    allocator->CreateBuffer(data, count * stride, usage, memtype, buffer, allocation, mapped);
    if(!buffer) return;
    this->count = count;
    this->stride = stride;
}

void CvkBuffer::Invalidate() {  //
    vmaInvalidateAllocation(*allocator, allocation, 0, VK_WHOLE_SIZE);
}

void CvkBuffer::Flush() {  // call after writing to mapped memory
    vmaFlushAllocation(*allocator, allocation, 0, VK_WHOLE_SIZE);
}

VkDeviceAddress CvkBuffer::DeviceAddress() {
    VkBufferDeviceAddressInfo addrInfo = {VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO};
    addrInfo.buffer = buffer;
    return vkGetBufferDeviceAddress(allocator->device, &addrInfo);
}
//----------------------------------------------------
//--------------------------VBO-----------------------
VBO::VBO(const void* data, uint32_t count, uint32_t stride) {Data(data, count, stride);}

void VBO::Data(const void* data, uint32_t count, uint32_t stride) {  // does NOT pack normals
    CvkBuffer::Data(data, count, stride, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VMA_MEMORY_USAGE_GPU_ONLY);
}

// Convert vec3 to VK_FORMAT_A2B10G10R10_SNORM_PACK32
static uint32_t Pack32(vec3 v) {  //10bit per channel
    //v.normalize();
    int xi = (int)(v.x * 0x7FC00000);  xi=(xi>>22)&0x3FF;
    int yi = (int)(v.y * 0x7FC00000);  yi=(yi>>22)&0x3FF;
    int zi = (int)(v.z * 0x7FC00000);  zi=(zi>>22)&0x3FF;
    return (uint32_t)(xi<<20 | yi<<10 | zi);
}

void VBO::Data(const Vertex* verts, uint32_t count) {
    if(!allocator) allocator = default_allocator;
    if(allocator->pack_normals) {                          // Pack normal into 32-bits
        struct VertPack {vec3 pos; uint npack; vec2 tc;};  // 24-bytes per vertex
        std::vector<VertPack> vpack(count);                // VertPackArray
        repeat(count) {
            const Vertex& v = verts[i];
            uint32_t npack = Pack32(v.nrm);
            vpack[i] = {v.pos, npack, v.tc};
        }
        Data(vpack.data(), (uint32_t)vpack.size(), sizeof(VertPack));
    } else {
        Data(verts, count, sizeof(Vertex));  // dont pack
    }
}

void VBO::Data(const VertsArray& verts) {
    Data(verts.data(), verts.size());
}
//----------------------------------------------------
//--------------------------IBO-----------------------
IBO::IBO(const uint16_t* data, uint32_t count) {Data(data, count);}
IBO::IBO(const uint32_t* data, uint32_t count) {Data(data, count);}

void IBO::Data(const uint16_t* data, uint32_t count) {
    CvkBuffer::Data(data, count, 2, VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VMA_MEMORY_USAGE_GPU_ONLY);
}

void IBO::Data(const uint32_t* data, uint32_t count) {
    CvkBuffer::Data(data, count, 4, VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VMA_MEMORY_USAGE_GPU_ONLY);
}

void IBO::Data(const IndexArray& index) {
    Data(index.data(), (uint32_t)index.size());
}
//----------------------------------------------------
//--------------------------UBO-----------------------
UBO::UBO(VkDeviceSize size, uint32_t count) {Allocate(size, count);}
void UBO::Allocate(VkDeviceSize size, uint32_t count) {
    VkDeviceSize blocksize = RoundUp(size, 0x100);  // memory alignment: 256 bytes
    Data(0, count, blocksize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU, &mapped);
}

void UBO::Set(const void* data, size_t size) {
    if(!count) Allocate(size);
    assert((stride == RoundUp(size, 0x100)) && (count==1));
    Update(data);
}

void UBO::Update(const void* data) {
    index = (index + 1) % count;
    Update(data, index);
    //printf("index=%d\n",index);
}

void UBO::Update(const void* data, uint32_t index) {
    ASSERT(index < count, "UBO array index out of bounds.\n");
    if(fence) VKERRCHECK(vkWaitForFences(allocator->device, 1, &fence, true, UINT64_MAX)) //wait for fence (if provided)
    //else VKERRCHECK(vkQueueWaitIdle(allocator->queue));                            //else wait for idle (slow)
    uint32_t offset = stride * index;
    memcpy((char*)mapped + offset, data, stride);
    vmaFlushAllocation(*allocator, allocation, offset, stride); //Flush();
}
//----------------------------------------------------
//--------------------------SBO-----------------------
void SBO::Allocate(VkDeviceSize size) {
    if(buffer) Clear();
    VkFlags usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT;
    CvkBuffer::Data(0, 1, size, usage, VMA_MEMORY_USAGE_GPU_ONLY);
}
//----------------------------------------------------
//--------------------------ABO-----------------------
ABO::ABO(VkDeviceSize size, ABOType type) {ABO::Allocate(size, type);}

void ABO::Allocate(VkDeviceSize size, ABOType type) {
    if(structure) Clear();
    VkFlags usage = VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT;
    Data(0, 1, size, usage, VMA_MEMORY_USAGE_GPU_ONLY);
    CreateAS(VkAccelerationStructureTypeKHR(type));
}

void ABO::Clear() {
    if(structure) {
        VKERRCHECK(vkQueueWaitIdle(allocator->queue));
        vkDestroyAccelerationStructureKHR(allocator->device, structure, nullptr);
        structure = nullptr;
    }
    CvkBuffer::Clear();
}

void ABO::CreateAS(VkAccelerationStructureTypeKHR type) { // Create BLAS/TLAS structure
    VkAccelerationStructureCreateInfoKHR createInfo{VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_CREATE_INFO_KHR};
    createInfo.type   = type; //VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR;
    createInfo.size   = stride;
    createInfo.buffer = buffer;
    VKERRCHECK(vkCreateAccelerationStructureKHR(allocator->device, &createInfo, nullptr, &structure));
}

void ABO::swap(ABO& other) {
    CvkBuffer::swap(other);
    std::swap(structure, other.structure);
}
//----------------------------------------------------

//---------------------YUVSampler---------------------
void YUVSampler::Create(VkDevice device) {
    GetProc(vkCreateSamplerYcbcrConversionKHR)
    if(!vkCreateSamplerYcbcrConversionKHR) return;
    this->device = device;
    VkSamplerYcbcrConversionCreateInfo ci = { VK_STRUCTURE_TYPE_SAMPLER_YCBCR_CONVERSION_CREATE_INFO };
    ci.format      = VK_FORMAT_G8B8G8R8_422_UNORM;
    ci.ycbcrModel  = VK_SAMPLER_YCBCR_MODEL_CONVERSION_YCBCR_709;
    ci.ycbcrRange  = VK_SAMPLER_YCBCR_RANGE_ITU_FULL;
    ci.xChromaOffset               = VK_CHROMA_LOCATION_MIDPOINT;
    ci.yChromaOffset               = VK_CHROMA_LOCATION_MIDPOINT;
    ci.chromaFilter                = VK_FILTER_LINEAR;
    ci.forceExplicitReconstruction = VK_FALSE;
    VKERRCHECK(vkCreateSamplerYcbcrConversionKHR(device, &ci, nullptr, &conversion));

    info = {VK_STRUCTURE_TYPE_SAMPLER_YCBCR_CONVERSION_INFO, 0, conversion};

    //VkSamplerCreateInfo
    VkSamplerCreateInfo samplerInfo = {VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO};
    samplerInfo.pNext = &info;
    samplerInfo.magFilter    = VK_FILTER_LINEAR;
    samplerInfo.minFilter    = VK_FILTER_LINEAR;
    samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    //samplerInfo.mipmapMode
    VKERRCHECK( vkCreateSampler(device, &samplerInfo, nullptr, &yuvSampler) );
}

void YUVSampler::Destroy() {
    if(conversion) vkDestroySamplerYcbcrConversion(device, conversion, 0); conversion = 0;
    if(yuvSampler) vkDestroySampler               (device, yuvSampler, 0); yuvSampler = 0;
}
//----------------------------------------------------

