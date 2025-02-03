// * Copyright (C) 2019 by Rene Lindsay

/*
*  CSwapchain uses CDepthBuffer to allocate a depth buffer for the swapchain framebuffer.
*  CSwapchain will automatically resize the depth buffer if the window gets resized.
*
*  CAllocator wraps the "Vulkan Memory Allocator" library. (First created CAllocator will be the default allocator)
*  Pass a CAllocator instance to the constructor of CvkBuffer and CvkImage objects. (or leave blank to use default allocator)
*
*  CvkBuffer is the base class for VBO, IBO and UBO classes.
*  Buffer types:
*      VBO : Vertex buffer object   : Array of structs (interleaved data)
*      IBO : Index buffer object    : Array of type uint16_t or uint32_t
*      UBO : Uniform buffer object  : One or more struct instances, with mapped memory
*
*  CvkImage creates a Vulkan image (texture) and  uploads data from CPU to GPU memory.
*  When creating a CvkImage, the input data may be one of the following types:
*  Image types:
*      RGBA      : Converts a color to a 1x1 texture
*      CImage    : A 2d-image of RGBA values
*      CCubemap  : A cubemap, used for skybox and environment map reflections
*      void*     : Specify format and extent manually
*
*  NOTE: "Vulkan Memory Allocator" fails to compile on VS2013. 
*        Therefore, minimum supported compiler is now VS2015.
*
*/

#ifndef BUFFERS_H
#define BUFFERS_H


#include "CDevices.h"
#include "vk_mem_alloc.h"
//#include "CImage.h"
#include "matrix.h"

#undef MOVE_SEMANTICS
//------------------------------------------------------------
#define MOVE_SEMANTICS(CLASS)                                \
    public:                                                  \
        CLASS(const CLASS&) = delete;                        \
        CLASS& operator=(const CLASS&) = delete;             \
        CLASS(CLASS&& other) noexcept { swap(other); }       \
        CLASS& operator=(CLASS&& other) noexcept {           \
            if(this != &other) swap(other);                  \
            return *this;                                    \
        }                                                    \
    protected:                                               \
        void swap(CLASS& other);
//------------------------------------------------------------

void setLayout(VkCommandBuffer cmd, VkImage image, VkImageLayout oldLayout, VkImageLayout newLayout, uint32_t baseMipLevel = 0, uint32_t mipLevels = VK_REMAINING_MIP_LEVELS, uint32_t layers = VK_REMAINING_ARRAY_LAYERS);

//------------------------------------vmaBuffer-----------------------------------
struct vmaBuffer {
    VmaAllocationInfo allocInfo =  {};
    VkBuffer      buffer      = VK_NULL_HANDLE;
    VmaAllocation bufferAlloc = VK_NULL_HANDLE;
    operator VkBuffer () {return buffer;}
    operator void* () {return allocInfo.pMappedData;}
};
//--------------------------------------------------------------------------------
//-----------------------------------YUVSampler-----------------------------------
class YUVSampler {
    VkDevice device = 0;
    VkSampler yuvSampler = 0;
    VkSamplerYcbcrConversion conversion = 0;
    void Destroy();
public:
    void Create(VkDevice device);
    ~YUVSampler(){Destroy();}
    operator VkSampler() const { return yuvSampler; }  
    VkSamplerYcbcrConversionInfo info;
};
//--------------------------------------------------------------------------------
//------------------------------------Allocator-----------------------------------
class CAllocator : public CCmd {
    VmaAllocator     allocator;
    VkInstance       instance;
    VkPhysicalDevice gpu;
    //VkDevice         device;
    //VkQueue          queue;
    //VkCommandPool    command_pool;
    //VkCommandBuffer  command_buffer;
    void BeginCmd(){CCmd::Begin();}
    void EndCmd(){CCmd::End(true);}

    void SetImageLayout(VkImage image, VkImageLayout oldLayout, VkImageLayout newLayout, uint32_t baseMipLevel = 0, uint32_t mipLevels = VK_REMAINING_MIP_LEVELS, uint32_t layers = VK_REMAINING_ARRAY_LAYERS);

    friend class CvkBuffer;
    friend class CvkImage;
    friend class CvkCubemap;
    friend class FBO;        // for ReadImage
    friend class Swapchain;  // for ReadImage
    friend class ABO;

    vmaBuffer vkmalloc(uint64_t size, VkBufferUsageFlags usage, bool mapped=true);
    void vkfree(vmaBuffer sb);

    void CreateBuffer(const void* data, uint64_t size, VkFlags usage, VmaMemoryUsage memtype, VkBuffer& buffer, VmaAllocation& alloc, void** mapped = 0); //usage of type VkBufferUsageFlags
    void DestroyBuffer(VkBuffer buffer, VmaAllocation alloc);
    
    void CreateImage(const void* data, VkExtent3D extent, VkFormat format, uint32_t mipLevels, VkImage& image, VmaAllocation& alloc, VkImageView& view);
    void CreateImage(const void* data, VkExtent3D extent, VkFormat format, uint32_t mipLevels, uint32_t arrayLayers, VkImageViewType viewType, VkImageUsageFlags usage, VkImage& image, VmaAllocation& alloc, VkImageView& view, void** mapped = 0);
    void CreateImage(VkExtent2D extent, VkFormat format, VkSampleCountFlagBits samples, VkImageUsageFlags usage, VkImage& image, VmaAllocation& alloc, VkImageView& view);  // For Swapchain Attachments
    void DestroyImage(VkImage image, VkImageView view, VmaAllocation alloc);
    bool GenerateMipmaps(VkImage image, VkFormat imageFormat, int32_t texWidth, int32_t texHeight, uint32_t mipLevels, uint32_t arrayLayers);
    void WriteImage(VkImage& image, VkImageLayout layout, VkExtent3D extent, VkFormat format, const void* data, uint32_t mipLevel=0, uint32_t mipLevels=1, uint32_t arrayLayers=1);
    void ReadImage (VkImage& image, VkImageLayout layout, VkExtent3D extent, VkFormat format, void* data);

public:
    size_t buf_stats = 0;  // total buffer memory allocated
    size_t img_stats = 0;  // total image memory allocated
    YUVSampler yuv_sampler;

    CAllocator();
    CAllocator(VkInstance instance, const CQueue& queue, VkDeviceSize blockSize=256);
    virtual ~CAllocator();
    void Init(VkInstance instance, const CQueue& queue, VkDeviceSize blockSize=256); // Select a transfer queue for staging operations.

    float maxAnisotropy = 1.0f;
    bool  useRTX        = false;
    bool  pack_normals  = false;
    std::vector<VmaBudget> GetBudget();
    operator VmaAllocator () {return allocator;}
};

extern CAllocator* default_allocator;  // Allow VBO/IBO/UBO/CvkImage to be used as member variables
//--------------------------------------------------------------------------------
//-------------------------------------Buffers------------------------------------
class CvkBuffer {
    MOVE_SEMANTICS(CvkBuffer)
    friend class BLAS;
    friend class VKRay;
protected:
    CAllocator*   allocator;
    VmaAllocation allocation;
    VkBuffer      buffer;
    uint32_t      count;
protected:
    VkDeviceSize  stride;
public:
    void* mapped = nullptr;

    CvkBuffer(CAllocator& allocator);
    CvkBuffer();
    ~CvkBuffer();
    virtual void Clear();
    void Invalidate();  // Invalidate cache before reading (only needed if not HOST_COHERENT)
    void Flush();       // Flush the buffer after writing  (only needed if not HOST_COHERENT)
    void Data(const void* data, uint32_t count, uint32_t stride, VkFlags usage, VmaMemoryUsage memtype=VMA_MEMORY_USAGE_GPU_ONLY, void** mapped = nullptr);
    uint32_t Count() { return count; }
    VkDeviceSize Stride(){ return stride; }
    VkDeviceSize size(){ return stride * count; }
    VkDeviceAddress DeviceAddress();  // Requires Vulkan 1.2
    operator VkBuffer () {return buffer;}
    operator VkBuffer* () {return &buffer;}
};

struct Vertex {vec3 pos; vec3 nrm; vec2 tc;};
//struct Vertex {vec3 pos; vec3 nrm; vec2 tc; vec4 col;};
typedef std::vector<Vertex>   VertsArray;
typedef std::vector<uint32_t> IndexArray;

class VBO : public CvkBuffer {  // Vertex buffer
public:
    using CvkBuffer::CvkBuffer;
    VBO() : CvkBuffer() {}
    VBO(const void* data, uint32_t count, uint32_t stride);
    void Data(const void* data, uint32_t count, uint32_t stride);  //make private for packed verts?
    void Data(const Vertex* verts, uint32_t count);
    void Data(const VertsArray& verts);
};

class IBO : public CvkBuffer {  // Index buffer
public:
    using CvkBuffer::CvkBuffer;
    IBO() : CvkBuffer() {}
    IBO(const uint16_t* data, uint32_t count);
    IBO(const uint32_t* data, uint32_t count);
    void Data(const uint16_t* data, uint32_t count);
    void Data(const uint32_t* data, uint32_t count);
    void Data(const IndexArray& index);
};

class UBO : public CvkBuffer {  // Uniform buffer
public:
    using CvkBuffer::CvkBuffer;
    UBO() : CvkBuffer() {}
    UBO(VkDeviceSize size, uint32_t count=1);
    void Set(const void* data, size_t size);
    void Allocate(VkDeviceSize size, uint32_t count=1);
    void Update(const void* data);                  // update entire buffer
    void Update(const void* data, uint32_t index);  // update a single item only
    uint32_t index=0;
    VkFence fence=0;  // optional, but may improve performance
};

class SBO : public CvkBuffer {  // scratch buffer
public:
    using CvkBuffer::CvkBuffer;
    //SBO() : CvkBuffer() {}
    SBO(VkDeviceSize size){Allocate(size);}
    void Allocate(VkDeviceSize size);
};

class ABO : public CvkBuffer {  // Acceleration Buffer (tlas/blas)
    MOVE_SEMANTICS(ABO)
    void CreateAS(VkAccelerationStructureTypeKHR type);
public:
    ABO() : CvkBuffer() {}
    enum ABOType {TLAS, BLAS};
    VkAccelerationStructureKHR structure = VK_NULL_HANDLE;
    using CvkBuffer::CvkBuffer;
    ABO(VkDeviceSize size, ABOType type);
    void Allocate(VkDeviceSize size, ABOType type);
    void Clear()  override;
    ~ABO(){Clear();}
};

//--------------------------------------------------------------------------------

#endif
