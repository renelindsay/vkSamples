//-------------------------------CImageBase---------------------------
// CImageBase:
// This is the base class for CImage, CImage32f and all other image formats.
// Assign the CImageBase to a vkImage, to upload the image to the GPU.
//
// CPack:
// Use CPack to compress or convert from CImage or CImage32f images,
// before uploading to the GPU. Various formats are supported.
// eg. BCn and YUV for LDR images, or RGB9E5 for HDR images.
// CPack returns the packed image as a CImageBase.
//--------------------------------------------------------------------

#include "Pack.h"
#include "CImage.h"
#include "Logging.h"
#include "matrix.h"

#define STB_DXT_IMPLEMENTATION
#include "stb_dxt.h"

#undef repeat
#define repeat(COUNT) for(uint32_t i = 0; i < (COUNT); ++i)
#define CLAMP(VAL, MIN, MAX) ((VAL<MIN)?MIN:(VAL>MAX)?MAX:VAL)
#define WRAP_INT(VAL, MOD) ((VAL%MOD+MOD)%MOD)
#define forXY(X, Y) for(int y = 0; y < (int)(Y); ++y) for(int x = 0; x < (int)(X); ++x)
//#define LERP(A, B, F) (F*(B-A)+A)

//---------------------CImageBase-------------------
void CImageBase::swap(CImageBase& other) {
    std::swap(buf,        other.buf);
    std::swap(size,       other.size);
    std::swap(width,      other.width);
    std::swap(height,     other.height);
    std::swap(magFilter,  other.magFilter);
    std::swap(wrapMode_U, other.wrapMode_U);
    std::swap(wrapMode_V, other.wrapMode_V);
    std::swap(colorspace, other.colorspace);
}

CImageBase::CImageBase(uint w, uint h, ImgFormat fmt) {
    switch (fmt) {
        case GRAY8       : SetSize(w,h, 8);  break;  //1
        case GRAY16      : SetSize(w,h,16);  break;  //2
        case R8G8B8      : SetSize(w,h,24);  break;  //3
        case R8G8BA8     : SetSize(w,h,32);  break;  //4
        case R4G4B4A4    : SetSize(w,h,16);  break;  //5
        case R5G6B5      : SetSize(w,h,16);  break;  //6
        case R8G8        : SetSize(w,h,16);  break;  //12
        case A2B10G10R10 : SetSize(w,h,32);  break;  //13
        case R16G16B16A16: SetSize(w,h,64);  break;  //14
        case R32G32B32A32: SetSize(w,h,128); break;  //15
        case RGB9E5      : SetSize(w,h,32);  break;  //17
        default : LOGE("Unsupported format."); return;
    }
    format = fmt;
}

void CImageBase::SetSize(uint w, uint h, uint bitspp) {
    width  = w;
    height = h;
    size   = (uint64_t)w*h*bitspp/8;
    if(size) { buf = realloc(buf, size); }
    else     { free(buf); buf = 0; }
    ASSERT(!!buf == !!size, "Failed to allocate image: (%dx%d) @ %dbpp\n",w,h,bitspp);
}

void CImageBase::Clear() {
    if(buf)memset(buf, 0, size);
}

void* CImageBase::Pixel(int x, int y) {
    ASSERT(format!=NONE, "Unknown format.\n");
    ASSERT(format<5, "Unsupported format.\n");  //only support formats 1,2,3,4
    // texture CLAMP vs WRAP
    if(wrapMode_U == wmREPEAT) x = WRAP_INT(x, width);
    if(wrapMode_V == wmREPEAT) y = WRAP_INT(y, height);
    x = CLAMP(x, 0, width -1);
    y = CLAMP(y, 0, height-1);
    int offs = (x + ((height-y-1) * width)) * format;
    return (char*)buf + offs;
}
//--------------------------------------------------

//-----------------------Pack-----------------------
Pack::Pack(CImage& img, ImgFormat dstFormat) {
    colorspace = img.colorspace;
    format = dstFormat;
    switch(dstFormat) {
        case GRAY8   :  toGray(img);         break;
        case R4G4B4A4:  to4444(img);         break;
        case R5G6B5  :  to565 (img);         break;
        case R8G8    :  to88  (img);         break;
        case BC1     :  toBC1 (img, false);  break;
        case BC1a    :  toBC1 (img, true);   break;
        case BC3     :  toBC3 (img);         break;
        case BC4     :  toBC4 (img);         break;
        case BC5     :  toBC4 (img);         break;
        case YUV422  :  toYUV (img);         break;
        default: break;
    }
}

Pack::Pack(CImage32f& img, ImgFormat dstFormat) {
    format = dstFormat;
    switch(dstFormat) {
        case RGB9E5       : toRGB9E5 (img);  break;
        case A2B10G10R10  : toRGB10  (img);  break;
        case R16G16B16A16 : toRGBA16f(img);  break;
        default: break;
    }
}

// Allocate buffer to match source image, and set block stride.
void Pack::BlockSize(const CImageBase& img, uint bitspp, uint blocksize) {
    CImageBase::SetSize(img.Width(), img.Height(), bitspp);
    b={width/blocksize, height/blocksize, bitspp*blocksize*blocksize/8};
}

void* Pack::Block(int x, int y) {  // Fetch compressed block
    int yf = b.h - 1 - y;  //flip
    uint64_t offs = (x+yf*b.w)*b.s;
    return (void*)((char*)buf + offs);
};

void Pack::toGray(CImage& img) {
    ASSERT(img.colorspace == csUNORM, "Can't convert sRGB to grayscale... Convert to UNORM first.\n");
    assert(img.colorspace == csUNORM);
    BlockSize(img, 8);
    forXY(b.w,b.h){*(uint8_t*)Block(x,y) = img.Pixel(x,y).toGray(); }
}

void Pack::to4444(CImage& img) {
    ASSERT(img.colorspace == csUNORM, "Can't convert sRGB to RGBA4444... Convert to UNORM first.\n");
    assert(img.colorspace == csUNORM);
    BlockSize(img, 16);
    forXY(b.w,b.h){*(uint16_t*)Block(x,y) = img.Pixel(x,y).to4444();}
}

void Pack::to565(CImage& img) {
    ASSERT(img.colorspace == csUNORM, "Can't convert sRGB to RGB565... Convert to UNORM first.\n");
    assert(img.colorspace == csUNORM);
    BlockSize(img, 16);
    forXY(b.w,b.h){*(uint16_t*)Block(x,y) = img.Pixel(x,y).to565();}
}

void Pack::to88(CImage& img) {
    assert(img.colorspace == csUNORM);
    BlockSize(img, 16);
    forXY(b.w,b.h){*(uint16_t*)Block(x,y) = img.Pixel(x,y).to88();}
}

void Pack::toRGB10(CImage32f& img) {  // RGB10
    BlockSize(img, 32);
    forXY(b.w,b.h){*(uint32_t*)Block(x,y) = img.Pixel(x,y).toRGB10();}
}

void Pack::toRGBA16f(CImage32f& img) {  // RGBA16f
    BlockSize(img, 64);
    forXY(b.w,b.h){*(uint64_t*)Block(x,y) = img.Pixel(x,y).toRGBA16f();}
}

void Pack::toRGB9E5(CImage32f& img) {  // 9bit RGB with 5bit shared exponent
    BlockSize(img, 32);
    forXY(b.w,b.h){*(uint32_t*)Block(x,y) = img.Pixel(x,y).toRGB9E5();}
}

void Pack::toYUV(CImage& img) {  // VK_FORMAT_G8B8G8R8_422_UNORM
     uint32_t w = img.Width();
     uint32_t h = img.Height();
     colorspace = csUNORM;
     CImageBase::SetSize(w, h, 16);
     uint8_t* buf = (uint8_t*)Buffer();
     uint prev=0;
     uint i = 0;
     bool sRGB = (img.colorspace==csSRGB);
     forXY(w,h) {
         YUV yuv = img.Pixel(x,h-y-1).toYUV(sRGB);
             buf[i++] = yuv.Y;
         if(x&1) {
             buf[i++] = (yuv.V+prev)/2;
             prev     = yuv.U;
         } else {
             buf[i++] = (yuv.U+prev)/2;
             prev     = yuv.V;
         }
     }
 }

// Fetch 4x4 tile of pixels from image
struct Tile4x4 {
    struct Line{RGBA pix[4];} line[4];
    Tile4x4(CImage& img, int x, int y) {
        line[0] = *(Line*)&img.Pixel(x*4, y*4+3);
        line[1] = *(Line*)&img.Pixel(x*4, y*4+2);
        line[2] = *(Line*)&img.Pixel(x*4, y*4+1);
        line[3] = *(Line*)&img.Pixel(x*4, y*4+0);
    }
    operator RGBA* (){return (RGBA*)line;}
};

void Pack::toBC1(CImage& img, bool alpha) {
    BlockSize(img, 4, 4);  // 4bpp 4x4
    forXY(b.w, b.h) {
        Tile4x4 tile4x4(img, x,y);  // grab 16 pixels from image (4x4)
        RGBA* pix = tile4x4;
        void* block = Block(x,y);
        stb_compress_dxt_block((uint8_t*)block, (uint8_t*)pix, 0, STB_DXT_HIGHQUAL);
        if(alpha) {                                                    // Flip block colors for 1-bit alpha
            uint32_t a=0; repeat(16) {a<<=2; a|=(pix[i].A==0)?3:0;}    // Mark alpha pixels in block...
            if(a) {                                                    // if found, flip to alpha mode
                uint16_t* s = (uint16_t*)block;                        // access block as 16bit shorts
                if(s[0]==s[1]) { s[0]&=0xFFFE;  s[1]|=0x0001; }        // ensure colors don't match
                else std::swap(s[0], s[1]);                            // swap colors to enable alpha mode
                uint32_t* block32 = (uint32_t*)block;                  // access block as 32bit ints
                const uint32_t b = block32[1];                         // get 2bit indexes
                const uint32_t c = ~(b|(b>>1));                        // set lower bit if index==0
                block32[1] = a | (b & 0xAAAAAAAA) | (c & 0x55555555);  // remap 2bit indexes (0123->1022)
            }
        }
    }
}

void Pack::toBC3(CImage& img, bool alpha) {
    if(!alpha) BlockSize(img, 4, 4);  // 4bpp 4x4  (BC1)
    else       BlockSize(img, 8, 4);  // 8bpp 4x4  (BC3)
    forXY(b.w, b.h) {
        Tile4x4 tile4x4(img, x,y);
        uint8_t* block = (uint8_t*)Block(x,y);
        stb_compress_dxt_block(block, (uint8_t*)tile4x4.line, alpha?1:0, STB_DXT_HIGHQUAL);
    }
}

void Pack::toBC4(CImage& img) {
    ASSERT(img.colorspace != csSRGB, "BC4 does not support sRGB images. (Use image.sRGBtoUNORM())\n");
    auto gray = img.asGray();

    BlockSize(gray, 4, 4);
    uint8_t* ibuf = (uint8_t*)gray.Buffer();

    auto imgPixel = [&](int x, int y) {
        int yf = height - 1 - y;  // flip
        uint64_t offs = (x+yf*width);
        return (void*)((char*)ibuf + offs);
    };

    forXY(b.w, b.h) {
        struct Line{uint8_t pix[4];};
        Line line[4] {  // read 4x4 block
            *(Line*)imgPixel(x*4, y*4+3),
            *(Line*)imgPixel(x*4, y*4+2),
            *(Line*)imgPixel(x*4, y*4+1),
            *(Line*)imgPixel(x*4, y*4+0)
        };
        uint8_t* block = (uint8_t*)Block(x,y);
        stb_compress_bc4_block(block, (uint8_t*)line);
    }
}

void Pack::toBC5(CImage& img) {
    ASSERT(img.colorspace != csSRGB, "BC5 does not support sRGB images. (Use image.sRGBtoUNORM())?\n");
    auto rg_img = img.as88();
    BlockSize(rg_img, 8, 4);
    uint16_t* ibuf = (uint16_t*)rg_img.Buffer();

    auto imgPixel = [&](int x, int y) {
        int yf = height - 1 - y;  // flip
        uint64_t offs = (x+yf*width)*2;
        return (void*)((char*)ibuf + offs);
    };

    forXY(b.w, b.h) {
        struct Line{uint16_t pix[4];};
        Line line[4] {  // read 4x4 block
            *(Line*)imgPixel(x*4, y*4+3),
            *(Line*)imgPixel(x*4, y*4+2),
            *(Line*)imgPixel(x*4, y*4+1),
            *(Line*)imgPixel(x*4, y*4+0)
        };
        uint8_t* block = (uint8_t*)Block(x,y);
        stb_compress_bc5_block(block, (uint8_t*)line);
    }
}

//--------------------------------------------------
