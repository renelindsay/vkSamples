#ifndef IMAGES_H
#define IMAGES_H

#include "Buffers.h"
#include "CImage.h"
#include "VkFormats.h"

#undef MOVE_SEMANTICS

//------------------------------------------------------------
#define MOVE_SEMANTICS(CLASS)                                \
    public:                                                  \
        CLASS(const CLASS&) = delete;                        \
        CLASS& operator=(const CLASS&) = delete;             \
        CLASS(CLASS&& other) : CLASS() { swap(other); }      \
        CLASS& operator=(CLASS&& other) {                    \
            if(this != &other) swap(other);                  \
            return *this;                                    \
        }                                                    \
    protected:                                               \
        void swap(CLASS& other);
//------------------------------------------------------------

//-------------------------------------Images-------------------------------------
//  Creates a Vulkan texture image, with option to generate mipmaps.
//  Input may be a color, 2d image, cubemap, or raw memory buffer

class CvkImage {
    MOVE_SEMANTICS(CvkImage)
    friend class Swapchain;
    friend class FBO;
    friend class CShader;  // for format
protected:
    CAllocator*           allocator;
    VmaAllocation         allocation;
    VkExtent3D            extent {};
    VkFormat              format = VK_FORMAT_UNDEFINED;
    VkImageLayout         layout = VK_IMAGE_LAYOUT_UNDEFINED;
    VkSampleCountFlagBits samples = VK_SAMPLE_COUNT_1_BIT;
    void CreateSampler(float maxLod = 0);
    VkFormatProperties FormatProperties(VkFormat fmt);
public:
    VkImage             image  = VK_NULL_HANDLE;
    VkImageView         view   = VK_NULL_HANDLE;
    VkSampler           sampler= VK_NULL_HANDLE;
    VkSamplerCreateInfo samplerInfo;  //Current sampler settings
    uint32_t            mipLevels=0;
    void*               mapped=0;

    CvkImage(CAllocator& allocator);
    CvkImage();
    CvkImage(RGBA color);
    CvkImage(CImage&     image, bool mipmap = false);
    CvkImage(CImage&     image, VkFormat format, bool mipmap = false);
    CvkImage(VkExtent2D extent, VkFormat format = VK_FORMAT_R8G8B8A8_UNORM, bool mipmap = false);
    CvkImage(const char* fname, VkFormat format = VK_FORMAT_R8G8B8A8_SRGB, bool mipmap = false);
    CvkImage(CImage32f&  image, VkFormat format = VK_FORMAT_R32G32B32A32_SFLOAT, bool mipmap = false);
    CvkImage(CCubemap& cubemap, VkFormat format = VK_FORMAT_R32G32B32A32_SFLOAT, bool mipmap = false);
    CvkImage(const void* data, VkExtent2D extent, VkFormat format = VK_FORMAT_R8G8B8A8_UNORM, bool mipmap = false);

    operator VkImage     () const {return image;}
    operator VkImageView () const {return view;}
    virtual ~CvkImage();

    CvkImage& operator = (const RGBA color)        { Data(color);   return *this; }
    CvkImage& operator = (      CImage& image)     { Data(image);   return *this; }
    CvkImage& operator = (      CCubemap& cubemap) { Data(cubemap); return *this; }

    VkExtent2D extent2D(){return {extent.width, extent.height};}
    //VkExtent3D extent3D(){return extent;}

    void Write(const void* data);  //Updates the image (dimensions stay the same)
    void Write(CImage& image);
    void MipTest();

    void Clear();
    void Data(const void* data, VkExtent3D extent, VkFormat format, bool mipmap, VkImageUsageFlags usage, bool ismapped = false);
    bool Data(const void* data, VkExtent2D extent, VkFormat format = VK_FORMAT_R8G8B8A8_UNORM, bool mipmap = false);
    void Data(const RGBA color);                              // Creates a 1x1 texture
    void Data(CImage& image, bool mipmap = false);
    void Data(CImage& image,     VkFormat   format, bool mipmap = false);
    void Data(const char* fname, VkFormat   format = VK_FORMAT_R8G8B8A8_SRGB,       bool mipmap = false);
    void Data(CImage32f& image,  VkFormat   format = VK_FORMAT_R32G32B32A32_SFLOAT, bool mipmap = false);
    void Data(CCubemap& cubemap, VkFormat   format = VK_FORMAT_R32G32B32A32_SFLOAT, bool mipmap = false);
    void Mapped(VkExtent2D extent, VkFormat format = VK_FORMAT_R8G8B8A8_UNORM);
    //---For Swapchain attachments---
    void SetSize(VkExtent2D extent, VkFormat format, VkSampleCountFlagBits samples, VkImageUsageFlags usage);
    void Resize(VkExtent2D extent, VkImageUsageFlags usage);
    //-------------------------------
    void SetSampler(VkSamplerCreateInfo& samplerInfo);  // Update sampler settings (TODO: vkUpdateDescriptorSets)
    void UpdateSampler() {SetSampler(samplerInfo);}
    CImage    Read();     // read image back from GPU to CPU memory
    CImage32f Read32f();  // read image back from GPU to CPU memory
    operator CImage    () { return this->Read(); }
    operator CImage32f () { return this->Read32f(); }

    void Blit(VkCommandBuffer cmd, VkImage dst, VkExtent2D ext, VkImageLayout dst_layout = VK_IMAGE_LAYOUT_GENERAL, VkFilter filter = VK_FILTER_LINEAR);

    void MinFilter(VkFilter minFilter = VK_FILTER_LINEAR);  // Call mesh.UpdateDescriptorSet(); to apply.
    void MagFilter(VkFilter magFilter = VK_FILTER_LINEAR);  // Call mesh.UpdateDescriptorSet(); to apply.

    void AddressModeU(VkSamplerAddressMode U  = VK_SAMPLER_ADDRESS_MODE_REPEAT);
    void AddressModeV(VkSamplerAddressMode V  = VK_SAMPLER_ADDRESS_MODE_REPEAT);
    //void AddressModeW(VkSamplerAddressMode W  = VK_SAMPLER_ADDRESS_MODE_REPEAT);

    VkImageLayout GetLayout() const {return layout;}
    void SetLayout(VkImageLayout newLayout);
    void PrintLayout();

    void* Pixel(int x, int y, uint8_t bytes_per_pixel=4);
    void Flush();
};

static VkExtent2D Extent2D(const CImageBase& img) { return {img.Width(), img.Height()}; };
static VkExtent3D Extent3D(const VkExtent2D ext2D){ return {ext2D.width, ext2D.height, 1}; };
//--------------------------------------------------------------------------------
//----------------------------------Depth Buffer----------------------------------
class CDepthBuffer : public CvkImage {
    VkImageUsageFlags usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT |
                              VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT;
public:
    void SetSize(VkExtent2D extent, VkFormat format = VK_FORMAT_D32_SFLOAT, VkSampleCountFlagBits samples = VK_SAMPLE_COUNT_1_BIT);
    void Resize(VkExtent2D extent);
};
//--------------------------------------------------------------------------------



#endif
