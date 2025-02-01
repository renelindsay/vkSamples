#pragma warning(disable: 4996)

#include "CImage.h"
#include "Logging.h"

#define STB_IMAGE_IMPLEMENTATION
#define STBI_NO_PSD
#define STBI_NO_GIF
#define STBI_NO_PIC
#define STBI_NO_PNM
#define STBI_NO_BMP
#include "stb_image.h"  // Loads: .jpg/.png/.hdr/.tga

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#include <stdlib.h>
#include <malloc.h>
#include <string.h>
#include <fstream> // for file_exists

#define STB_DXT_IMPLEMENTATION
#include "stb_dxt.h"

#undef repeat
#define repeat(COUNT) for(uint32_t i = 0; i < (COUNT); ++i)
#define CLAMP(VAL, MIN, MAX) ((VAL<MIN)?MIN:(VAL>MAX)?MAX:VAL)
#define WRAP_INT(VAL, MOD) ((VAL%MOD+MOD)%MOD)
#define forXY(X, Y) for(int y = 0; y < (int)(Y); ++y) for(int x = 0; x < (int)(X); ++x)
#define LERP(A, B, F) (F*(B-A)+A)

static bool file_exists(const char *fileName) {
    FILE* file = fopen(fileName, "r");
    if(file) { fclose(file); return true;}
    return false;
}

//-------------------RGBA----------------------
RGBA::RGBA() : R(0), G(0), B(0), A(255) {}
RGBA::RGBA(uint8_t val) : R(val), G(val), B(val), A(255) {}
RGBA::RGBA(uint8_t r, uint8_t g, uint8_t b, uint8_t a) : R(r), G(g), B(b), A(a) {}
void RGBA::set(uint8_t r, uint8_t g, uint8_t b, uint8_t a) { R=r; G=g; B=b; A=a;}

RGBA RGBA::lerp(RGBA c, float f) { 
    return RGBA( (uint8_t)(f*(c.R-R)+R),
                 (uint8_t)(f*(c.G-G)+G),
                 (uint8_t)(f*(c.B-B)+B),
                 (uint8_t)(f*(c.A-A)+A) );
}
//---------------------------------------------

//------------------RGBA32f--------------------
RGBA32f::RGBA32f() : R(0), G(0), B(0), A(1) {}
RGBA32f::RGBA32f(float r, float g, float b, float a) : R(r), G(g), B(b), A(a) {}
RGBA32f::RGBA32f(vec4 v) : R(v.x), G(v.y), B(v.z), A(v.w) {}
void RGBA32f::set(float r, float g, float b, float a) { R=r; G=g; B=b; A=a;}

#define USE_LUT
#ifndef USE_LUT
RGBA32f::RGBA32f(RGBA pix) {  // converts sRGB to linear
    float gamma = 2.2f;
    R = powf(pix.R / 255.f, gamma );
    G = powf(pix.G / 255.f, gamma );
    B = powf(pix.B / 255.f, gamma );
    A = pix.A / 255.f;
};

#else
RGBA32f::RGBA32f(RGBA pix) {  // converts sRGB to linear, using LUT (4x faster)
    // ----pow22-----
    static float pow22[256];  // powf(x, 2.2);  LUT to convert sRGB to linear float
    static struct _pow22{_pow22(){ repeat(256) pow22[i]=powf((float)i/255.f, 2.2f); } }_pow22; // build LUT
    //---------------

    R = pow22[pix.R];
    G = pow22[pix.G];
    B = pow22[pix.B];
    A = pix.A / 255.f;
};
#endif

RGBA32f::operator RGBA () const {  // converts linear to sRGB
    float inv_gamma = 1.f/2.2f;
    uint8_t r = (uint8_t)(powf(CLAMP(R,0, 1.f), inv_gamma) * 255);
    uint8_t g = (uint8_t)(powf(CLAMP(G,0, 1.f), inv_gamma) * 255);
    uint8_t b = (uint8_t)(powf(CLAMP(B,0, 1.f), inv_gamma) * 255);
    uint8_t a = (uint8_t)(A * 255);
    return RGBA(r,g,b,a); 
};

RGBA32f  RGBA32f::lerp(const RGBA32f& c, float f) { 
    return RGBA32f( (f*(c.R-R)+R), (f*(c.G-G)+G), (f*(c.B-B)+B), (f*(c.A-A)+A) );
}
//---------------------------------------------


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
        case GRAY8    : SetSize(w,h, 8); break;  //1
        case GRAY16   : SetSize(w,h,16); break;  //2
        case R8G8B8   : SetSize(w,h,24); break;  //3
        case R8G8BA8  : SetSize(w,h,32); break;  //4
        case R4G4B4A4 : SetSize(w,h,16); break;  //5
        case R5G6B5   : SetSize(w,h,16); break;  //6
        //case BC1      : SetSize(w,h, 4); break;
        //case BC1a     : SetSize(w,h, 4); break;
        //case BC3      : SetSize(w,h, 8); break;
        //case BC4      : SetSize(w,h, 4); break;
        //case BC5      : SetSize(w,h, 8); break;
        case R8G8     : SetSize(w,h,16); break;
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
//---------------------CImage-----------------------
RGBA& CImage::Pixel(int x, int y) {
    // texture CLAMP vs WRAP
    if(wrapMode_U == wmREPEAT) x = WRAP_INT(x, width);
    if(wrapMode_V == wmREPEAT) y = WRAP_INT(y, height);
    x = CLAMP(x, 0, width -1);
    y = CLAMP(y, 0, height-1);
    return Buffer()[x + ((height-y-1) * width)];
}

RGBA CImage::UV(float u, float v) {
    if(magFilter == NEAREST) return Pixel((int)(u * width), (int)(v * height));
    else                     return UV_bilinear(u, v);
}

RGBA CImage::UV_bilinear(float u, float v) {  // untested, and ignores gamma
    float fx = u * width;
    float fy = v * height;
    float ix = floor(fx);
    float iy = floor(fy);
    fx -= ix;
    fy -= iy;
    int x = (int)ix;
    int y = (int)iy;
    RGBA a = Pixel(x + 0, y + 0);
    RGBA b = Pixel(x + 1, y + 0);
    RGBA c = Pixel(x + 0, y + 1);
    RGBA d = Pixel(x + 1, y + 1);
    RGBA ab  = a.lerp(b, fx);
    RGBA cd  = c.lerp(d, fx);
    return ab.lerp(cd, fy);
}

void CImage::Clear(RGBA color) {
    repeat((uint)width * height) Buffer()[i] = color;
}

void CImage::Clear(RGBA color, bool R, bool G, bool B, bool A) {
    repeat((uint)width * height) {
        if(R) Buffer()[i].R = color.R;
        if(G) Buffer()[i].G = color.G;
        if(B) Buffer()[i].B = color.B;
        if(A) Buffer()[i].A = color.A;
    }
}

bool CImage::Load(const char* filename, bool flip) {
    if(!file_exists(filename)) { LOGE("CImage: File not found: %s\n", filename);  return false; }
    int w, h, n;
    stbi_set_flip_vertically_on_load(flip);
    char* tmp=(char*)stbi_load(filename, &w, &h, &n, sizeof(RGBA));
    if(tmp) {
        LOGI("Load image: %s (%dx%d)\n", filename, w, h);
        SetSize(w, h);
        memcpy(buf,tmp, (uint)w * (uint)h * sizeof(RGBA));
        stbi_image_free(tmp);
    } else LOGE("CImage: Failed to load texture.");
    return !!tmp;
}

bool CImage::Load_from_mem(const void* addr, int len, bool flip) {
    //int w, h, n;
    int w = 0, h = 0, n = 0;
    stbi_set_flip_vertically_on_load(flip);
    char* tmp=(char*)stbi_load_from_memory((const stbi_uc*)addr, len, &w, &h, &n, sizeof(RGBA));
    if(tmp) {
        SetSize(w, h);
        memcpy(buf, tmp, (uint)w * (uint)h * sizeof(RGBA));
        stbi_image_free(tmp);
    } else LOGE("CImage: Failed to load texture.");
    return !!tmp;
}

void CImage::write_pgm(const char* filename, bool sRGB) {
    CImageBase gray = asGray(sRGB ? csSRGB : csUNORM);
    FILE* file = fopen(filename, "wb");
    ASSERT(!!file, "CImage: Failed to write pgm file.\n");
    if(sRGB) fprintf(file, "P5\n# Using sRGB gamma transfer function. (gamma=2.2)\n");
    else     fprintf(file, "P5\n# Using LINEAR gamma transfer function. (No Gamma)\n");
    fprintf(file, "%d %d\n%d\n", width, height, 255);
    fwrite(gray.Buffer(), 1, width*height, file);
    fclose(file);
}

void CImage::Save(const char* filename) {
    printf("Saving file: %s\n", filename);
    const char* ext = strrchr(filename, '.');
    if(!strcmp(ext, ".jpg")) stbi_write_jpg(filename, width, height, sizeof(RGBA), buf, 80);
    if(!strcmp(ext, ".png")) stbi_write_png(filename, width, height, sizeof(RGBA), buf, 0);
    if(!strcmp(ext, ".tga")) stbi_write_tga(filename, width, height, sizeof(RGBA), buf);
    if(!strcmp(ext, ".pgm"))      write_pgm(filename, colorspace==csSRGB);
}

void CImage::Blend(CImage& other, float R_lerp, float G_lerp, float B_lerp) {
    forXY(width, height) {
        RGBA32f pix1 = Pixel(x,y);
        RGBA32f pix2 = other.UV((float)x/width,(float)y/height);
        float r = LERP(pix1.R, pix2.R, R_lerp);
        float g = LERP(pix1.G, pix2.G, G_lerp);
        float b = LERP(pix1.B, pix2.B, B_lerp);
        Pixel(x,y) = RGBA32f(r,g,b);
    }
}

// Undo jpg gamma correction. (Makes mid-tones darker)
// Allows sRGB texture to be loaded as UNORM.
// Warning: This loses some color fidelity.
// If possible, set Vulkan to sample the texture as sRGB instead.
void CImage::sRGBtoUNORM(float brightness, float contrast) {
    if(colorspace == csUNORM) {LOGW("sRGBtoUNORM: Image is already in UNORM colorspace.\n");}  // Warn if already UNORM
    if(brightness == 0.0f && contrast == 1.0f) {  //faster
        forXY(width, height) {
            RGBA32f pix = Pixel(x,y);               // sRGB -> linear float
            Pixel(x,y) = RGBA((uint8_t)(pix.R*255),
                              (uint8_t)(pix.G*255),
                              (uint8_t)(pix.B*255),
                              (uint8_t)(pix.A*255));  // linear -> UNORM
        }
    } else {
        forXY(width, height) {
            RGBA32f pix = Pixel(x,y);
            pix *= contrast;
            pix += brightness;
            uint8_t R = (uint8_t)(CLAMP(pix.R, 0, 1.f)*255);
            uint8_t G = (uint8_t)(CLAMP(pix.G, 0, 1.f)*255);
            uint8_t B = (uint8_t)(CLAMP(pix.B, 0, 1.f)*255);
            uint8_t A = (uint8_t)(CLAMP(pix.A, 0, 1.f)*255);
            Pixel(x,y) = RGBA(R,G,B,A);            // linear -> UNORM

        }
    }
    colorspace = csUNORM;
}

void CImage::UNORMtoSRGB() {
    if(colorspace == csSRGB) {LOGW("UNORMtoSRGB: Image is already in sRGB colorspace.\n"); }  // Warn if already sRGB
    float inv_gamma = 1.f/2.2f;
    forXY(width, height){
        RGBA32f pix;
        pix.setFromUNORM(Pixel(x,y));
        uint8_t r = (uint8_t)(powf(CLAMP(pix.R, 0, 1.f), inv_gamma) * 255);
        uint8_t g = (uint8_t)(powf(CLAMP(pix.G, 0, 1.f), inv_gamma) * 255);
        uint8_t b = (uint8_t)(powf(CLAMP(pix.B, 0, 1.f), inv_gamma) * 255);
        uint8_t a = (uint8_t)(pix.A * 255);
        //Pixel(x,y) = {r, g, b, a};
        Pixel(x,y).set(r,g,b,a);
    }
    colorspace = csSRGB;
}

void CImage::BGRAtoRGBA() {  // swap Red and Blue
    //LOGV("Perf warning: Calling BGRAtoRGBA()\n");
    forXY(width, height) {
        RGBA& pix = Pixel(x,y);
        pix=RGBA(pix.B, pix.G, pix.R, pix.A);
    }
}

CImageBase CImage::asGray(ColorSpace cs) {  // High quality, but slow.  sRGB or UNORM
    float gamma = 1.0f/2.2f;
    CImageBase gray(width, height, GRAY8);  // Create blank grayscale image
    CImage32f img32 = *this;                // Convert to linear float, and undo gamma, if any.
    forXY(width, height) {
        float lum = img32.Pixel(x,y).Luminocity();
        if(cs==csSRGB) lum = powf(lum, gamma );
        char* pix = (char*)gray.Pixel(x,y);
        *pix = (uint8_t)CLAMP(lum, 0.f, 1.f)*255;
    }
    gray.colorspace = cs;
    return gray;
}

//  Copy and convert from 1/2/3/4 Bpp format buffer(addr), to RGBA(local).
void CImage::Copy(const void* addr, int bpp, bool Vflip) {
    forXY(width, height) {
        RGBA& pix = Pixel(x,y);
        int yf = Vflip?(height - 1 - y) : y;
        char* ptr = (char*)addr+(x + yf * width) * bpp;
        switch (bpp) {
            case 1: pix = RGBA(ptr[0]);                         break;  // gray
            case 2: pix = RGBA(ptr[1]);                         break;  // gray16
            case 3: pix = RGBA(ptr[0], ptr[1], ptr[2]);         break;  // RGB
            case 4: pix = RGBA(ptr[0], ptr[1], ptr[2], ptr[3]); break;  // RGBA
            default : pix = RGBA();
        }
    }
}
//--------------------------------------------------
//---------------------CImage32f--------------------
CImage32f::CImage32f(CImage& img) {
    SetSize(img.Width(), img.Height());
    if(img.colorspace==csSRGB)
          forXY(width, height) Pixel(x, y) = img.Pixel(x, y);              //from sRGB image
    else  forXY(width, height) Pixel(x, y).setFromUNORM(img.Pixel(x, y));  //from UNORM image
}

RGBA32f& CImage32f::Pixel(int x, int y) {
    // texture CLAMP vs WRAP
    if(wrapMode_U == wmREPEAT) x = WRAP_INT(x, width);
    if(wrapMode_V == wmREPEAT) y = WRAP_INT(y, height);
    x = CLAMP(x, 0, width -1);
    y = CLAMP(y, 0, height-1);
    return Buffer()[x + ((height-y-1) * width)];
}

RGBA32f CImage32f::UV(float u, float v) {  // Nearest filtering. TODO: Trilinear / Anisotropic
    if(magFilter == NEAREST) return Pixel((int)(u * width), (int)(v * height));
    else                     return UV_bilinear(u, v);
}

RGBA32f CImage32f::UV_bilinear(float u, float v) {  // untested
    float fx = u * width;
    float fy = v * height;
    float ix = floor(fx);
    float iy = floor(fy);
    fx -= ix;
    fy -= iy;
    int x = (int)ix;
    int y = (int)iy;
    RGBA32f a = Pixel(x + 0, y + 0);
    RGBA32f b = Pixel(x + 1, y + 0);
    RGBA32f c = Pixel(x + 0, y + 1);
    RGBA32f d = Pixel(x + 1, y + 1);
    RGBA32f ab  = a.lerp(b, fx);
    RGBA32f cd  = c.lerp(d, fx);
    return ab.lerp(cd, fy);
}

void CImage32f::Clear(RGBA32f color) {
    repeat((uint32_t)width * height) Buffer()[i] = color;
}

void CImage32f::Bleed(uint32_t margin) {
    CImage32f img(width, height);
    repeat(margin) {
        forXY(width, height) img.Pixel(x,y) = Pixel(x,y);  //copy image
        forXY(width, height) {
            RGBA32f& pix = Pixel(x,y);
            if(pix.A <= 0) {
                RGBA32f* p[9] = {
                    &img.Pixel(x-1,y-1), &img.Pixel(x+0,y-1), &img.Pixel(x+1,y-1),
                    &img.Pixel(x-1,y+0), &img.Pixel(x+0,y+0), &img.Pixel(x+1,y+0),
                    &img.Pixel(x-1,y+1), &img.Pixel(x+0,y+1), &img.Pixel(x+1,y+1)
                };
                uint ctr=0;
                RGBA32f accum {};
                repeat(9) if (p[i]->A >= 1) {accum += *p[i]; ctr++;}
                if(ctr) {
                    accum /= (float)ctr;
                    Pixel(x,y) = accum;
                }
            }
        }
    }
}

void CImage32f::Blur() {
    CImage32f img(width, height);
    forXY(width, height) img.Pixel(x,y) = Pixel(x,y);  //copy image
    forXY(width, height) {
        RGBA32f* p[9] = {
            &img.Pixel(x-1,y-1), &img.Pixel(x+0,y-1), &img.Pixel(x+1,y-1),
            &img.Pixel(x-1,y+0), &img.Pixel(x+0,y+0), &img.Pixel(x+1,y+0),
            &img.Pixel(x-1,y+1), &img.Pixel(x+0,y+1), &img.Pixel(x+1,y+1)
        };
        RGBA32f accum = *p[4] * 0.25f;                      // center
        accum += (*p[1] + *p[3] + *p[5] + *p[7]) * 0.125;   // sides
        accum += (*p[0] + *p[2] + *p[6] + *p[8]) * 0.0625;  // corners
        Pixel(x,y) = accum;
    }
}
/*
void CImage32f::toGray() {  // not used
    forXY(width, height) {
        RGBA32f pix = Pixel(x,y);
        float gray = pix.Luminocity();
        pix.set(gray, gray, gray, pix.A);
    }
}
*/
/*
//   Finds the brightest spot on a skysphere
//   texture, and returns its direction vector. (untested)

vec3 CImage32f::GetSunVec() {
    float max = 0;
    vec2i xy;
    forXY(width, height) {   // find the brightest point
        float lum = Pixel(x,y).Luminocity();
        if(lum>max) {xy = vec2i(x,y); max = lum;}
    }
    float pitch = 90.f - (xy.y/height)*180.f;
    float yaw   = (xy.x/width)*360.f;
    vec3 v(0,0,1);
    v.RotateX(pitch);
    v.RotateY(yaw);
    return v;
}
*/

CImage CImage32f::toLDR(float brightness, float contrast, float gamma) {
    float inv_gamma = 1.f/gamma;
    CImage ldr(width, height);
    if(gamma==1.0) ldr.colorspace = csUNORM;
    forXY(width, height) {
        RGBA32f pix = Pixel(x,y) * contrast;
        pix += {brightness, brightness, brightness, 1};
        uint8_t r = (uint8_t)(powf(CLAMP(pix.R,0, 1.f), inv_gamma) * 255);
        uint8_t g = (uint8_t)(powf(CLAMP(pix.G,0, 1.f), inv_gamma) * 255);
        uint8_t b = (uint8_t)(powf(CLAMP(pix.B,0, 1.f), inv_gamma) * 255);
        uint8_t a = (uint8_t)(pix.A * 255);
        ldr.Pixel(x,y) = {r, g, b, a};
    }
    return ldr;
}

bool CImage32f::Load(const char* filename, bool flip) {
    if(!file_exists(filename)) { LOGE("CImage32f: File not found: %s\n", filename);  return false; }

    int w, h, n;
    stbi_set_flip_vertically_on_load(flip);
    float* tmp = stbi_loadf(filename, &w, &h, &n, 4);
    if(tmp) {
        LOGI("Load image: %s (%dx%d)\n", filename, w, h);
        SetSize(w, h);
        memcpy(buf,tmp, (uint)w * (uint)h * sizeof(RGBA32f));
        stbi_image_free(tmp);
    } else LOGE("CImage32f: Failed to load texture.");
    return !!tmp;
}

void CImage32f::Save(const char* filename) {
    printf("Saving file: %s\n", filename);
    const char* ext = strrchr(filename, '.');
    ASSERT(!strcmp(ext, ".hdr"), "When saving an image from CImage32f, use .hdr format.\n" );
    stbi_write_hdr(filename, width, height, 4, (const float*)buf);
}
//--------------------------------------------------
//---------------------CConvert---------------------
Convert::Convert(CImage& img, ImgFormat dstFormat) {
    colorspace = img.colorspace;
    format = dstFormat;
    switch(dstFormat) {
        case GRAY8   :  toGray(img);                 break;
        case R4G4B4A4:  to4444(img);                 break;
        case R5G6B5  :  to565 (img);                 break;
        case R8G8    :  to88  (img);                 break;
        case BC1     :  toBC1 (img, false);          break;
        case BC1a    :  toBC1 (img, true);           break;
        case BC3     :  toDXT (img);                 break;
        case BC4     :  toBC4(Convert(img, GRAY8));  break;
        case BC5     :  toBC5(Convert(img, R8G8 ));  break;
        default: break;
    }
}

Convert::Convert(CImageBase& img, ImgFormat dstFormat) {
    format = dstFormat;
    switch(dstFormat) {
        case BC4 : toBC4(img); break;
        case BC5 : toBC5(img); break;
        default: break;
    }
}


// Allocate buffer to match source image, and set block stride.
void Convert::BlockSize(const CImageBase& img, uint bitspp, uint blocksize) {
    CImageBase::SetSize(img.Width(), img.Height(), bitspp);
    b={width/blocksize, height/blocksize, bitspp*blocksize*blocksize/8};
}

void* Convert::Block(int x, int y) {  // Fetch compressed block
    int yf = b.h - 1 - y;  //flip
    uint64_t offs = (x+yf*b.w)*b.s;
    return (void*)((char*)buf + offs);
};

void Convert::toGray(CImage& img) {
    ASSERT(img.colorspace == csUNORM, "Can't convert sRGB to grayscale... Convert to UNORM first.\n");
    assert(img.colorspace == csUNORM);
    BlockSize(img, 8);
    forXY(b.w,b.h){*(uint8_t*)Block(x,y) = img.Pixel(x,y).toGray(); }
}

void Convert::to4444(CImage& img) {
    ASSERT(img.colorspace == csUNORM, "Can't convert sRGB to RGBA4444... Convert to UNORM first.\n");
    assert(img.colorspace == csUNORM);
    BlockSize(img, 16);
    forXY(b.w,b.h){*(uint16_t*)Block(x,y) = img.Pixel(x,y).to4444();}
}

void Convert::to565(CImage& img) {
    ASSERT(img.colorspace == csUNORM, "Can't convert sRGB to RGB565... Convert to UNORM first.\n");
    assert(img.colorspace == csUNORM);
    BlockSize(img, 16);
    forXY(b.w,b.h){*(uint16_t*)Block(x,y) = img.Pixel(x,y).to565();}
}

void Convert::to88(CImage& img) {
    assert(img.colorspace == csUNORM);
    BlockSize(img, 16);
    forXY(b.w,b.h){*(uint16_t*)Block(x,y) = img.Pixel(x,y).to88();}
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

void Convert::toBC1(CImage& img, bool alpha) {
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

void Convert::toDXT(CImage& img, bool alpha) {
    if(!alpha) BlockSize(img, 4, 4);  // 4bpp 4x4  (BC1)
    else       BlockSize(img, 8, 4);  // 8bpp 4x4  (BC3)
    forXY(b.w, b.h) {
        Tile4x4 tile4x4(img, x,y);
        uint8_t* block = (uint8_t*)Block(x,y);
        stb_compress_dxt_block(block, (uint8_t*)tile4x4.line, alpha?1:0, STB_DXT_HIGHQUAL);
    }
}

void Convert::toBC4(const CImageBase& gray) {
    ASSERT(gray.colorspace != csSRGB, "BC4 does not support sRGB images. (Use image.sRGBtoUNORM())\n");
    ASSERT(gray.format == GRAY8,      "BC4 can only compress grayscale images.\n");
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

void Convert::toBC5(const CImageBase& rg_img) {
    ASSERT(rg_img.colorspace != csSRGB, "BC5 does not support sRGB images. (Use image.sRGBtoUNORM())?\n");
    ASSERT(rg_img.format  == R8G8,      "BC5 can only compress R8G8 images.\n");
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
