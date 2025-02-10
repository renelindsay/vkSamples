#pragma warning(disable: 4996)

#include "CImage.h"
#include "Logging.h"
#include <cmath>

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
//#include <fstream> // for file_exists

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

YUV RGBA::toYUV (bool isSRGB) {  // sRGB to YUV
    float r = R;
    float g = G;
    float b = B;
    if(isSRGB) {
        RGBA32f f(*this); // sRGB to UNORM
        r = f.R*255;
        g = f.G*255;
        b = f.B*255;
    }
    float y = 0.299f * r + 0.587f * g + 0.114f * b;
  //float u =-0.1687 * r - 0.3313 * g + 0.5000 * b;
  //float v = 0.5000 * r - 0.4187 * g - 0.0813 * b;
    float u =(0.492f * ((float)b - y));
    float v =(0.877f * ((float)r - y));
    uint8_t Y = clampi(y    ,0,255);
    uint8_t U = clampi(u+128,0,255);
    uint8_t V = clampi(v+128,0,255);
    return {Y,U,V};
}

RGBA RGBA::lerp(RGBA c, float f) {
    return RGBA( (uint8_t)(f*(c.R-R)+R),
                 (uint8_t)(f*(c.G-G)+G),
                 (uint8_t)(f*(c.B-B)+B),
                 (uint8_t)(f*(c.A-A)+A) );
}
//---------------------------------------------

//------------------RGBA32f--------------------
//RGBA32f::RGBA32f() : R(0), G(0), B(0), A(1) {}
RGBA32f::RGBA32f(float r, float g, float b, float a) : R(r), G(g), B(b), A(a) {}
RGBA32f::RGBA32f(vec4 v) : R(v.x), G(v.y), B(v.z), A(v.w) {}
RGBA32f::RGBA32f(RGBA16f pix) : R(pix.R), G(pix.G), B(pix.B), A(pix.A) {}
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

uint32_t RGBA32f::toRGB9E5() {
    float r = clampf(R, 0, 65408);
    float g = clampf(G, 0, 65408);
    float b = clampf(B, 0, 65408);
    float max_c = std::max(r, std::max(g,b));
    int shift = std::ceil(std::log2(max_c*512))-9;
    shift = clampi(shift, -15, 16);
    float mul = powf(2, 9-shift);
    uint32_t test = max_c * mul + 0.5f;  // check for overflow
    if(test>=512 && shift<16) {++shift;  mul/=2;}
    uint32_t R9 = r * mul + 0.5f;
    uint32_t G9 = g * mul + 0.5f;
    uint32_t B9 = b * mul + 0.5f;
    uint32_t E5 = 15 + shift;
    return (E5 << 27) | (B9 << 18) | (G9 << 9) | (R9 << 0);
}

RGBA32f  RGBA32f::lerp(const RGBA32f& c, float f) { 
    return RGBA32f( (f*(c.R-R)+R), (f*(c.G-G)+G), (f*(c.B-B)+B), (f*(c.A-A)+A) );
}
//---------------------------------------------

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

CImage CImage::Mipmap() {
    //auto pixel = [&](int x, int y) {return Buffer()[x + y * width];};
    uint w = width/2;
    uint h = height/2;
    CImage img(w, h);
    img.colorspace = colorspace;
    forXY(w, h) {
        uint x2 = x * 2;
        uint y2 = y * 2;
        RGBA32f a = Pixel(x2 + 0, y2 + 0);
        RGBA32f b = Pixel(x2 + 1, y2 + 0);
        RGBA32f c = Pixel(x2 + 0, y2 + 1);
        RGBA32f d = Pixel(x2 + 1, y2 + 1);
        RGBA32f col = (a + b + c + d) / 4.f;
        img.Pixel(x,y) = col;
    }
    return img;
}

bool CImage::Load(const char* filename, bool flip) {
    if(!file_exists(filename)) { LOGE("CImage: File not found: %s\n", filename);  return false; }
    int w, h, n;
    stbi_set_flip_vertically_on_load(flip);
    char* tmp=(char*)stbi_load(filename, &w, &h, &n, sizeof(RGBA));
    if(tmp) {
        LOGI("Load image: %s (%dx%d)\n", filename, w, h);
        SetSize(w, h);
        memcpy(buf, tmp, (uint)w * (uint)h * sizeof(RGBA));
        stbi_image_free(tmp);
    } else LOGE("CImage: Failed to load texture.");
    return !!tmp;
}

bool CImage::Load_from_mem(const void* addr, int len, bool flip) {
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
    CImageBase gray = asGrayHQ(sRGB ? csSRGB : csUNORM);
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

CImageBase CImage::asGrayHQ(ColorSpace cs) {  // High quality, but slow.  sRGB or UNORM
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

RGBA32f CImage32f::UV(float u, float v) {  // Nearest or Bilinear filtering. TODO: Trilinear / Anisotropic
    if(magFilter == NEAREST) return Pixel((int)(u * width), (int)(v * height));
    else                     return UV_bilinear(u, v);
}

RGBA32f CImage32f::UV_bilinear(float u, float v) {
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

CImage32f CImage32f::Mipmap() {
    //auto pixel = [&](int x, int y) {return Buffer()[x + y * width];};
    uint w = width/2;
    uint h = height/2;
    CImage32f img(w, h);
    forXY(w, h) {
        uint x2 = x * 2;
        uint y2 = y * 2;
        RGBA32f a = Pixel(x2 + 0, y2 + 0);
        RGBA32f b = Pixel(x2 + 1, y2 + 0);
        RGBA32f c = Pixel(x2 + 0, y2 + 1);
        RGBA32f d = Pixel(x2 + 1, y2 + 1);
        RGBA32f col = (a + b + c + d) / 4.f;
        img.Pixel(x,y) = col;
    }
    return img;
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
    ivec2 xy(width, height);
    forXY(width, height) {   // find the brightest point
        float lum = Pixel(x,y).Luminocity();
        if(lum>max) {xy = ivec2(x,y); max = lum;}
    }
    float pitch = 90.f - ((float)xy.y/height)*180.f;
    float yaw   = ((float)xy.x/width)*360.f;
    vec3 v(0,0,1);
    v.RotateX(pitch);
    v.RotateY(yaw);
    return v;
}
*/
ivec2 CImage32f::GetBrightestPixel() {
    float max = 0;
    ivec2 xy;
    forXY(width, height) {   // find the brightest point
        float lum = Pixel(x,y).Luminocity();
        if(lum>max) {xy = ivec2(x,y); max = lum;}
    }
    return xy;
}


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

//---------------------Cubemap----------------------
// Convert from equirectangular to rectilinear
void CCubemap::LoadPanorama(const char* filename, MagFilter filter) {
    CImage32f pan;
    pan.Load(filename, true);
    ASSERT(pan.Width() == pan.Height()*2, "Panorama image should have a 2:1 size ratio.");
    pan.wrapMode_U = wmREPEAT;
    pan.magFilter  = filter; // LINEAR / NEAREST
    int w = pan.Width() / 4;
    for(auto& f:face) f.SetSize(w, w);
    vec3 v0(-1, 1, 1);
    vec3 v1( 1, 1, 1);
    vec3 v2(-1,-1, 1);
    vec3 v3( 1,-1, 1);
    const float degToU = 1.f/360.f;
    const float degToV = 1.f/180.f;
    for(int y = 0; y<w; ++y) {
        float fy = (float)y / w;
        vec3 v02 = v0.lerp(v2, fy);
        vec3 v13 = v1.lerp(v3, fy);
        for(int x = 0; x<w; ++x) {
            float fx = (float)x / w;
            vec3 v = v02.lerp(v13, fx);
            euler e = v;
            float px = e.yaw  * degToU;
            float py = e.pitch* degToV;
            face[FRONT].Pixel(x,y) = pan.UV(px      , py);
            face[LEFT ].Pixel(x,y) = pan.UV(px+0.25f, py);
            face[BACK ].Pixel(x,y) = pan.UV(px+0.50f, py);
            face[RIGHT].Pixel(x,y) = pan.UV(px+0.75f, py);
            euler e2 = vec3(-v.y,-v.z, v.x);
            px = e2.yaw  * degToU;
            py = e2.pitch* degToV;
            face[BOTTOM].Pixel(x,y) = pan.UV(px ,py);
            face[TOP   ].Pixel(x,y) = pan.UV(1.f-px ,1.f-py);
        }
    }
}

// Returns the direction of the brightest point on the skybox.
vec3 CCubemap::GetSunVec(RGBA32f* flux) {
    float max = 0;
    uint faceid=0;
    float hw = face[0].Width()/2;
    float x = 0;
    float y = 0;
    float z = hw;
    RGBA32f color;
    repeat(6) {
        ivec2 p = face[i].GetBrightestPixel();
        RGBA32f col = face[i].Pixel(p.x, p.y);
        float lum = col.Luminocity();
        if(lum>max) {x=p.x; y=p.y; max=lum; color=col; faceid=i;}
    }
    x -= hw-0.5f;
    y -= hw-0.5f;
    if(flux) *flux = color;
    //printf("Sun vec: x=%f y=%f z=%f\n", x,y,z);
    switch(faceid) {
        case 0 : return vec3( z, y,-x).normalized();
        case 1 : return vec3(-z, y, x).normalized();
        case 2 : return vec3( x, z,-y).normalized();
        case 3 : return vec3( x,-z, y).normalized();
        case 4 : return vec3( x, y, z).normalized();
        case 5 : return vec3(-x, y,-z).normalized();
        default : return {0,0,0};
    }
}
//--------------------------------------------------
