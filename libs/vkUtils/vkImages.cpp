#include "vkImages.h"
#include <functional>
#include <math.h>

//uint32_t Log2(uint32_t x) {return (uint32_t)(log(x) / log(2));}

//-----------------------CvkImage---------------------
CvkImage::CvkImage(CAllocator& allocator)                           : allocator(&allocator), allocation(), extent(), format(), samplerInfo() {}
//CvkImage::CvkImage()                                                : allocator(default_allocator), allocation(), samplerInfo(), extent(), format() { }  //{ Data(RGBA()); }
CvkImage::CvkImage(RGBA color)                                      : allocator(default_allocator), allocation() { Data(color); }
CvkImage::CvkImage(CImage& image,                      bool mipmap) : allocator(default_allocator), allocation() { Data(image, mipmap); }
CvkImage::CvkImage(CImage& image,     VkFormat format, bool mipmap) : allocator(default_allocator), allocation() { Data(image, format, mipmap); }
CvkImage::CvkImage(CCubemap& cubemap, VkFormat format, bool mipmap) : allocator(default_allocator), allocation() { Data(cubemap, format, mipmap); }
CvkImage::CvkImage(VkExtent2D extent, VkFormat format, bool mipmap) : allocator(default_allocator), allocation() { Data(0, extent, format, mipmap); }
CvkImage::CvkImage(const char* fname, VkFormat format, bool mipmap) : allocator(default_allocator), allocation() { Data(fname,format, mipmap); }
CvkImage::CvkImage(CImage32f& image,  VkFormat format, bool mipmap) : allocator(default_allocator), allocation() { Data(image, format, mipmap); }
CvkImage::CvkImage(const void* data, VkExtent2D extent, VkFormat format, bool mipmap) : allocator(default_allocator), allocation() { Data(data, extent, format, mipmap); }

CvkImage::CvkImage() : allocator(default_allocator), allocation(), extent(), format(), samplerInfo() { }

CvkImage::~CvkImage() { Clear(); allocator = 0; }

void CvkImage::Clear() {
    //ASSERT(!!default_allocator, "No GPU memory allocator found.\n");
    //if(format==VK_FORMAT_UNDEFINED) return;
    if(allocator) {
        VKERRCHECK(vkQueueWaitIdle(allocator->queue));
        if(sampler) vkDestroySampler(allocator->device, sampler, nullptr);
        if(image) allocator->DestroyImage(image, view, allocation);
    } else allocator = default_allocator;
    sampler = 0;
    image   = 0;
    view    = 0;
    extent  = {};
    format  = VK_FORMAT_UNDEFINED;
    layout  = VK_IMAGE_LAYOUT_UNDEFINED;
    samples = VK_SAMPLE_COUNT_1_BIT;
}

// move construction
void CvkImage::swap(CvkImage& other) {
    std::swap(allocator,   other.allocator);
    std::swap(allocation,  other.allocation);
    std::swap(image,       other.image);
    std::swap(view,        other.view);
    std::swap(sampler,     other.sampler);

    std::swap(extent,      other.extent);
    std::swap(format,      other.format);
    std::swap(samplerInfo, other.samplerInfo);
    std::swap(mapped,      other.mapped);
}

void CvkImage::Write(const void* data) {
    //VkImageLayout layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    allocator->WriteImage(image, layout, extent, format, data, 0, mipLevels, 1);  // updates mips too
}

void CvkImage::Write(CImage& image) {
    ASSERT(extent.width==image.Width() && extent.height==image.Height(), "vkImage.Update: Dimensions don't match existing image.\n");
    Write(image.Buffer());
}

// Sets each mipmap level to a different color, so you can see which LOD is active.
// Create an image of say 256x256, and call MipTest() to color code mip-levels.
void CvkImage::MipTest() {
    VkImageLayout layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    CImage img0(extent.width/1,  extent.height/1,  {255,0,0,255});
    allocator->WriteImage(image, layout, extent, format, img0, 0, 1, 1);

    CImage img1(extent.width/2,  extent.height/2,  {0,255,0,255});
    allocator->WriteImage(image, layout, extent, format, img1, 1, 1, 1);

    CImage img2(extent.width/4,  extent.height/4,  {0,0,255,255});
    allocator->WriteImage(image, layout, extent, format, img2, 2, 1, 1);

    CImage img3(extent.width/8,  extent.height/8,  {128,0,0,255});
    allocator->WriteImage(image, layout, extent, format, img3, 3, 1, 1);

    CImage img4(extent.width/16, extent.height/16, {0,128,0,255});
    allocator->WriteImage(image, layout, extent, format, img4, 4, 1, 1);

    CImage img5(extent.width/32, extent.height/32, {0,0,128,255});
    allocator->WriteImage(image, layout, extent, format, img5, 5, 1, 1);
}

void CvkImage::Data(const void* data, VkExtent3D extent, VkFormat format, bool mipmap, VkImageUsageFlags usage, bool ismapped) {
    //ASSERT(this->extent.width==0, "Image already loaded.");
    Clear();
    ASSERT(!!allocator, "VMA Allocator not initialized.");
    mipLevels = 1;
    if(mipmap) {
        mipLevels = (uint32_t)(log2(std::max(extent.width, extent.height))) + 1;
        if(FormatInfo(format).isCompressed()) mipLevels-=2;  // BC1 uses blocks of 4x4
    }
    allocator->CreateImage(data, extent, format, mipLevels, 1, VK_IMAGE_VIEW_TYPE_2D, usage, image, allocation, view, ismapped ? &mapped : 0);
    if(!image) return;
    this->format = format;
    this->extent = extent;
    this->layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    CreateSampler((float)mipLevels);
}

bool CvkImage::Data(const void* data, VkExtent2D extent, VkFormat format, bool mipmap) {
    VkExtent3D extent3D = {extent.width, extent.height, 1};
    VkImageUsageFlags usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT |
                              VK_IMAGE_USAGE_TRANSFER_DST_BIT |
                              VK_IMAGE_USAGE_SAMPLED_BIT;
    Data(data, extent3D, format, mipmap, usage);
    return((!mipmap) || !(FormatProperties(format).optimalTilingFeatures & VK_FORMAT_FEATURE_BLIT_DST_BIT));
}

// Get format from image, if not specified
void CvkImage::Data(CImage& image, bool mipmap) {
    VkExtent2D ext = Extent2D(image);
    if(image.colorspace == csUNORM) Data(image, ext, VK_FORMAT_R8G8B8A8_UNORM, mipmap);
    if(image.colorspace == csSRGB ) Data(image, ext, VK_FORMAT_R8G8B8A8_SRGB, mipmap);
}

//  Converts RGBA image to specified format
void CvkImage::Data(CImage& image, VkFormat format, bool mipmap) {
    format_type ftype = FormatInfo(format).type;
    if(image.colorspace == csSRGB && ftype==UNORM) LOGW("Using sRGB image as a UNORM texture.\n");
    if(image.colorspace == csUNORM && ftype==SRGB) LOGW("Using UNORM image as a SRGB texture.\n");
    if(mipmap) ASSERT(format!=VK_FORMAT_G8B8G8R8_422_UNORM, "YUV image format does not support mipmaps.\n");

    VkExtent2D ext = Extent2D(image);
    std::function<CImageBase(CImage&)> pack = 0;  //[](CImage& img){return std::move(img);};
    switch (format) {
        case VK_FORMAT_R8G8B8A8_UNORM        :
        case VK_FORMAT_R8G8B8A8_SRGB         : Data(image, ext, format, mipmap); return;
        case VK_FORMAT_R8_UNORM              :
        case VK_FORMAT_R8_SRGB               : pack = &CImage::asGray;  break;
        case VK_FORMAT_R8G8_UNORM            :
        case VK_FORMAT_R8G8_SRGB             : pack = &CImage::as88;    break;
        case VK_FORMAT_R4G4B4A4_UNORM_PACK16 : pack = &CImage::as4444;  break;
        case VK_FORMAT_R5G6B5_UNORM_PACK16   : pack = &CImage::as565;   break;
        case VK_FORMAT_BC1_RGB_UNORM_BLOCK   :
        case VK_FORMAT_BC1_RGB_SRGB_BLOCK    : pack = &CImage::asBC1;   break;
        case VK_FORMAT_BC1_RGBA_UNORM_BLOCK  :
        case VK_FORMAT_BC1_RGBA_SRGB_BLOCK   : pack = &CImage::asBC1A;  break;
        case VK_FORMAT_BC3_UNORM_BLOCK       :
        case VK_FORMAT_BC3_SRGB_BLOCK        : pack = &CImage::asBC3;   break;
        case VK_FORMAT_BC4_UNORM_BLOCK       : pack = &CImage::asBC4;   break;
        case VK_FORMAT_BC5_UNORM_BLOCK       : pack = &CImage::asBC5;   break;
        case VK_FORMAT_G8B8G8R8_422_UNORM    : pack = &CImage::asYUV;   break;
        default : LOGE("CvkImage: Format not supported");
    }
    bool need_mips = Data(pack(image), ext, format, mipmap);

    // --- If GPU fails to generate mipmaps, use CPU instead. ---
    if(need_mips) {
        LOGV("Generate mipmaps on CPU(%d)\n", mipLevels);
        CImage mip = image.Mipmap();
        repeat(mipLevels-1) {
            if(i) mip = mip.Mipmap();
            allocator->WriteImage(this->image, layout, extent, format, pack(mip), 1+i);
        }
    }
    //-----------------------------------------------------------
}

void CvkImage::Data(RGBA color) {
    if(!allocator) { LOGW("CvkImage has no allocator.\n"); return;}
    CImage colorImg(1,1,color);
    //colorImg.sRGBtoUNORM();
    colorImg.colorspace = csUNORM;
    Data(colorImg);
}

void CvkImage::Data(const char* fname, VkFormat format, bool mipmap) {
    CImage image(fname);
    Data(image, format, mipmap);
}

void CvkImage::Data(CImage32f& image, VkFormat format, bool mipmap) {
    VkExtent2D ext = Extent2D(image);
    std::function<CImageBase(CImage32f&)> pack = 0;
    switch (format) {
        case VK_FORMAT_E5B9G9R9_UFLOAT_PACK32   : pack = &CImage32f::asRGB9E5;  break;
        case VK_FORMAT_A2B10G10R10_UNORM_PACK32 : pack = &CImage32f::asRGB10;   break;
        case VK_FORMAT_R16G16B16A16_SFLOAT      : pack = &CImage32f::asRGBA16f; break;
        case VK_FORMAT_R32G32B32A32_SFLOAT      : Data(image, ext, format, mipmap); return;
        default : LOGE("CImage32f: Cubemap format not supported.");
    }
    bool need_mips = Data(pack(image), ext, format, mipmap);
    if(need_mips) {
        LOGV("Generate mipmaps on CPU(%d)\n", mipLevels);
        CImage32f mip = image.Mipmap();
        repeat(mipLevels-1) {
            if(i) mip = mip.Mipmap();
            allocator->WriteImage(this->image, layout, extent, format, pack(mip), 1+i);
        }
    }
}

void CvkImage::Data(CCubemap& cubemap, VkFormat format, bool mipmap) {
    Clear();
    ASSERT(!!allocator, "VMA Allocator not initialized.");
    repeat(6) assert(cubemap.face[i].Width() == cubemap.face[i].Height() && "Cubemap faces must have square images.");
    repeat(5) assert(cubemap.face[i].Width() == cubemap.face[i+1].Width() && "Cubemap faces must have matching sizes.");

    uint32_t w = cubemap.face[0].Width();
    uint32_t h = cubemap.face[0].Height();
    extent = {w,h,1};
    uint32_t mipLevels = 1;
    if(mipmap) mipLevels = (uint32_t)(log2(w)) + 1;

    VkImageUsageFlags usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT |
                              VK_IMAGE_USAGE_TRANSFER_DST_BIT |
                              VK_IMAGE_USAGE_SAMPLED_BIT;

    auto info = FormatInfo(format);
    uint32_t size = info.size * w * h;
    void* buf = malloc(size * 6);

    std::function<CImageBase(CImage32f&)> pack = 0;
    switch (format) {
        case VK_FORMAT_E5B9G9R9_UFLOAT_PACK32   : pack = &CImage32f::asRGB9E5;  break;
        case VK_FORMAT_A2B10G10R10_UNORM_PACK32 : pack = &CImage32f::asRGB10;   break;
        case VK_FORMAT_R16G16B16A16_SFLOAT      : pack = &CImage32f::asRGBA16f; break;
        case VK_FORMAT_R32G32B32A32_SFLOAT      : pack = [](CImage32f& img){return std::move(img);}; break;
        default : LOGE("CImage32f: Cubemap format not supported. (Use: R32G32B32A32, R16G16B16A16, A2B10G10R10, or E5B9G9R9)\n");
    }

    repeat(6) memcpy((char*)buf+(size*i), pack(cubemap.face[i]), size);
    allocator->CreateImage(buf, extent, format, mipLevels, 6, VK_IMAGE_VIEW_TYPE_CUBE, usage, image, allocation, view);

    // ---- If GPU failed to generate Mipmaps, use CPU instead. ----
    auto props = FormatProperties(format);
    if(mipmap && !(props.optimalTilingFeatures & VK_FORMAT_FEATURE_BLIT_DST_BIT)) { // check if format supports blit    
        LOGV("Generate mipmaps on CPU(%d)\n", mipLevels);
        CImage32f f[6];
        repeat(6) f[i] = cubemap.face[i].Mipmap();
        repeat(mipLevels-1) {
            if(i) repeat(6) f[i] = f[i].Mipmap();
            int fsize = f[0].Width() *f[0].Height() * 4;
            repeat(6) memcpy((char*)buf+(fsize*i), pack(f[i]), fsize);
            allocator->WriteImage(image, layout, extent, format, buf, 1+i, 1, 6);
        }
    }
    // -------------------------------------------------------------

    free(buf);
    this->format = format;
    this->layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    CreateSampler((float)mipLevels);
}

VkFormatProperties CvkImage::FormatProperties(VkFormat fmt) {
    VkPhysicalDevice gpu = allocator->gpu;
    VkFormatProperties formatProperties;
    vkGetPhysicalDeviceFormatProperties(gpu, fmt, &formatProperties);
    return formatProperties;
}

void CvkImage::Mapped(VkExtent2D extent, VkFormat format) {
    Clear();
    VkExtent3D extent3D = {extent.width, extent.height, 1};
    VkImageUsageFlags usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT |
                              VK_IMAGE_USAGE_TRANSFER_DST_BIT |
                              VK_IMAGE_USAGE_SAMPLED_BIT;
    Data(0, extent3D, format, false, usage, true);
}

void CvkImage::SetSize(VkExtent2D extent, VkFormat format, VkSampleCountFlagBits samples, VkImageUsageFlags usage) {
    Clear();
    ASSERT(!!allocator, "VMA Allocator not initialized.");
    allocator->CreateImage(extent, format, samples, usage, image, allocation, view);
    this->format  = format;
    this->samples = samples;
    this->extent  = Extent3D(extent);
    this->layout  = VK_IMAGE_LAYOUT_UNDEFINED;
    CreateSampler(0);  // not used?
}

void CvkImage::Resize(VkExtent2D extent, VkImageUsageFlags usage) {
    ASSERT(!!image, "Image not initialized\n");
    SetSize(extent, format, samples, usage);
}

void CvkImage::CreateSampler(float maxLod) {
    //VkSamplerCreateInfo 
    samplerInfo = {VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO};
    samplerInfo.magFilter    = VK_FILTER_LINEAR;
    samplerInfo.minFilter    = VK_FILTER_LINEAR;
    //samplerInfo.magFilter    = VK_FILTER_NEAREST;
    //samplerInfo.minFilter    = VK_FILTER_NEAREST;
    samplerInfo.mipmapMode   = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.mipLodBias   = 0;
    //---Anisotropy---
    float aniso=allocator->maxAnisotropy; // 1-16
    samplerInfo.maxAnisotropy    = aniso;
    samplerInfo.anisotropyEnable = aniso>1.f ? VK_TRUE : VK_FALSE;
    //----------------
    samplerInfo.compareEnable = VK_FALSE;
    samplerInfo.compareOp     = VK_COMPARE_OP_ALWAYS;
    samplerInfo.minLod        = 0;
    samplerInfo.maxLod        = maxLod; //mipLevels;
    //samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
    samplerInfo.unnormalizedCoordinates = VK_FALSE;
    SetSampler(samplerInfo);
} 

void CvkImage::SetSampler(VkSamplerCreateInfo& samplerInfo) {
    if(sampler) vkDestroySampler(allocator->device, sampler, nullptr);
    VKERRCHECK( vkCreateSampler(allocator->device, &samplerInfo, nullptr, &sampler) );
}
//----------------------------------------------------
CImage CvkImage::Read() {  //Read image from GPU.
    format_info fmt = FormatInfo(format);
    CImage img(extent.width, extent.height);
    if(format == VK_FORMAT_R32G32B32A32_SFLOAT) {
        CImage32f img32f(extent.width, extent.height);
        allocator->ReadImage(image, layout, extent, format, img32f);
        img = img32f.toLDR();
        LOGV("Reading CImage32f as CImage. (Converting to sRGB)\n");
    } else
    if(format == VK_FORMAT_R16G16B16A16_SFLOAT) {
        int w = extent.width;
        int h = extent.height;
        RGBA16f* img16f = (RGBA16f*)malloc(w * h * 8);
        allocator->ReadImage(image, layout, extent, format, img16f);
        for(int y=0; y<h; ++y) {
            RGBA16f* in_line = &img16f[y*w];
            RGBA* out_line = &img[y*w];
            for(int x=0; x<w; ++x) {
                RGBA32f pix32f = in_line[x];
                out_line[x] = pix32f;
            }
        }
        free(img16f);
    } else
    if(fmt.size == 4) {
        if(fmt.type==UNORM) img.colorspace = csUNORM;
        if(fmt.type==SRGB)  img.colorspace = csSRGB;
        if(fmt.isSwizzled())img.BGRAtoRGBA();
        //VkImageLayout layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        allocator->ReadImage(image, layout, extent, format, img);
    } else LOGE("CvkImage.Read() failed to read image. (Unsupported format)\n");
    return img;
}

CImage32f CvkImage::Read32f() {
    //format_info fmt = FormatInfo(format);
    ASSERT(format == VK_FORMAT_R32G32B32A32_SFLOAT, "Read32f: Source image is not VK_FORMAT_R32G32B32A32_SFLOAT\n");
    CImage32f img32f(extent.width, extent.height);
    allocator->ReadImage(image, layout, extent, format, img32f);
    return img32f;
}
//----------------------------------------------------

void CvkImage::Blit(VkCommandBuffer cmd, VkImage dst, VkExtent2D ext, VkImageLayout dst_layout, VkFilter filter) {
    VkImageSubresourceLayers subresourceLayers = {};
    subresourceLayers.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    subresourceLayers.mipLevel = 0;
    subresourceLayers.baseArrayLayer = 0;
    subresourceLayers.layerCount = 1;

    VkImageBlit region = {};
    region.srcSubresource = subresourceLayers;
    region.srcOffsets[1] = {(int)extent.width, (int)extent.height, 1};
    region.dstSubresource = subresourceLayers;
    region.dstOffsets[1] = {(int)ext.width, (int)ext.height, 1};

    VkImageLayout blit_layout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    //VkImageLayout blit_layout = VK_IMAGE_LAYOUT_GENERAL;
    setLayout(cmd, dst, VK_IMAGE_LAYOUT_UNDEFINED, blit_layout);
    vkCmdBlitImage(cmd, image, layout, dst, blit_layout, 1, &region, filter);
    setLayout(cmd, dst, blit_layout, dst_layout);
}

//----------------------------------------------------

void CvkImage::MinFilter(VkFilter minFilter) { samplerInfo.minFilter = minFilter; UpdateSampler(); }
void CvkImage::MagFilter(VkFilter magFilter) { samplerInfo.magFilter = magFilter; UpdateSampler(); }

void CvkImage::AddressModeU(VkSamplerAddressMode U) { samplerInfo.addressModeU = U; UpdateSampler(); }
void CvkImage::AddressModeV(VkSamplerAddressMode V) { samplerInfo.addressModeV = V; UpdateSampler(); }
//void CvkImage::AddressModeW(VkSamplerAddressMode W) { samplerInfo.addressModeV = W; UpdateSampler(); }

void CvkImage::SetLayout(VkImageLayout newLayout) {
    allocator->SetImageLayout(image, layout, newLayout, 0);  // uses Cmd
    layout = newLayout;
}

void CvkImage::PrintLayout() {
#if !defined(NDEBUG)
    const char* str = "";
    switch (layout) {
#define STR(r) case r: str = #r; break;
        STR(VK_IMAGE_LAYOUT_UNDEFINED);
        STR(VK_IMAGE_LAYOUT_GENERAL);
        STR(VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
        STR(VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
        STR(VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL);
        STR(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
        STR(VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
        STR(VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
        STR(VK_IMAGE_LAYOUT_PREINITIALIZED);
        STR(VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_STENCIL_ATTACHMENT_OPTIMAL);
        STR(VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_STENCIL_READ_ONLY_OPTIMAL);
        STR(VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL);
        STR(VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_OPTIMAL);
        STR(VK_IMAGE_LAYOUT_STENCIL_ATTACHMENT_OPTIMAL);
        STR(VK_IMAGE_LAYOUT_STENCIL_READ_ONLY_OPTIMAL);
        STR(VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);
        STR(VK_IMAGE_LAYOUT_SHARED_PRESENT_KHR);
        STR(VK_IMAGE_LAYOUT_SHADING_RATE_OPTIMAL_NV);
        STR(VK_IMAGE_LAYOUT_FRAGMENT_DENSITY_MAP_OPTIMAL_EXT);
        STR(VK_IMAGE_LAYOUT_READ_ONLY_OPTIMAL_KHR);
        STR(VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL_KHR);
    default : str = "UNKNOWN";
#undef STR
    }
    printf("ImageLayout=%s\n", str);
#endif
}

void* CvkImage::Pixel(int x, int y, uint8_t bytes_per_pixel) {
    ASSERT(!!mapped, "Can't access pixel data. Image was not mapped to be host-visible.");
    int w = extent.width;
    int h = extent.height;
    x = clampi(x, 0, w-1);
    y = clampi(y, 0, h-1);
    if(w%16) w+=16-(w%16);  // mem alignment
    int offs = (x + ((h-y-1) * w)) * bytes_per_pixel;
    return (char*)mapped + offs;
}

void CvkImage::Flush() {  // call after writing to mapped memory
    vmaFlushAllocation(*allocator, allocation, 0, VK_WHOLE_SIZE);
}


//------------------------Depth Buffer------------------------
void CDepthBuffer::SetSize(VkExtent2D extent, VkFormat format, VkSampleCountFlagBits samples) {
    CvkImage::SetSize(extent, format, samples, usage);
}

void CDepthBuffer::Resize(VkExtent2D extent) {
    CvkImage::Resize(extent, usage);
}
//------------------------------------------------------------

