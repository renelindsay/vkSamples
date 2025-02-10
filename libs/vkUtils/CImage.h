//----------------------------CImage---------------------------------
//  Use the CImage class to load an image from file or memory buffer.
//  Files may be in jpg, png or hdr format.
//  Use CCubemap to load 6 sides of a cubemap texture.
//  Pass CImage or CCubemap to CvkImage, to upload the image to Vulkan.
//
//  Notes:
//    CImage32f images are always in linear color space
//    CImage may be in sRGB or UNORM(linear) color space. (default=sRGB)
//    To save VRAM, CImage can be used to compress images to BCn format.
//--------------------------------------------------------------------

#ifndef CIMAGE_H
#define CIMAGE_H

#include <stdint.h>
#include <stdio.h>
#include <malloc.h>
#include "matrix.h"
#include "fp16.h"
#include "Pack.h"

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
    private:                                                 \
        void swap(CLASS& other);
//------------------------------------------------------------

typedef uint32_t uint;
typedef unsigned char u_char;
typedef unsigned short ushort;

//-------------------------------------------------------------------------------
// 8 bit per channel sRGB (gamma)
struct RGB {
    uint8_t R=0, G=0, B=0;
};
//-------------------------------------------------------------------------------
// YUV
struct YUV {
    uint8_t Y=0, U=0, V=0;
};
//-------------------------------------------------------------------------------
// 8 bit per channel sRGBA (gamma or linear)
struct RGBA { 
    uint8_t R=0, G=0, B=0, A=0;
    RGBA();
    RGBA(uint8_t val);
    RGBA(RGB rgb) {R=rgb.R; G=rgb.G; B=rgb.B; A=255;}
    RGBA(uint8_t r, uint8_t g, uint8_t b, uint8_t a=255);
    void set(uint8_t r, uint8_t g, uint8_t b, uint8_t a=255);
    RGBA lerp(RGBA c, float f);

    u_char toGray(){return (u_char)(R*0.3f + G*0.59f + B*0.11f);}
    ushort to4444(){return R>>4<<12 | G>>4<<8 | B>>4<<4 | A>>4;}
    ushort to565 (){return R>>3<<11 | G>>2<<5 | B>>3;}
    ushort to88  (){return *(ushort*)(this);}
    YUV    toYUV (bool isSRGB=true);
};
//-------------------------------------------------------------------------------
// 16bit per channel float (linear)
struct RGBA16f {
    fp16 R, G, B, A;
    RGBA16f() : R(0), G(0), B(0), A(1) {};
    RGBA16f(float r, float g, float b, float a=1) : R(r), G(g), B(b),A(a) {};
    operator uint64_t () {return *(uint64_t*)this;}
};
//-------------------------------------------------------------------------------
// 32bit per channel float (linear)
struct RGBA32f { 
    float R, G, B, A;
    RGBA32f(): R(0), G(0), B(0), A(1) {};
    RGBA32f(float r, float g, float b, float a=1);
    RGBA32f(RGBA pix);                   // converts sRGB to linear
    RGBA32f(vec4 col);
    RGBA32f(RGBA16f pix);
    operator RGBA () const;              // converts linear to sRGB
    RGBA32f& operator += (const RGBA32f& c) { R+=c.R;  G+=c.G;  B+=c.B; A=1; return *this; }
    RGBA32f& operator -= (const RGBA32f& c) { R-=c.R;  G-=c.G;  B-=c.B; A=1; return *this; }
    RGBA32f& operator *= (const RGBA32f& c) { R*=c.R;  G*=c.G;  B*=c.B; A=1; return *this; }
    RGBA32f& operator /= (const RGBA32f& c) { R/=c.R;  G/=c.G;  B/=c.B; A=1; return *this; }
    RGBA32f  operator + (const RGBA32f& c) const { return {R+c.R, G+c.G, B+c.B, 1}; }
    RGBA32f  operator - (const RGBA32f& c) const { return {R-c.R, G-c.G, B-c.B, 1}; }
    RGBA32f  operator * (const RGBA32f& c) const { return {R*c.R, G*c.G, B*c.B, 1}; }
    RGBA32f  operator / (const RGBA32f& c) const { return {R/c.R, G/c.G, B/c.B, 1}; }
    RGBA32f& operator *= (float f) { R*=f;  G*=f;  B*=f;  return *this; }
    RGBA32f& operator /= (float f) { R/=f;  G/=f;  B/=f;  return *this; }
    RGBA32f& operator += (float f) { R+=f;  G+=f;  B+=f;  return *this; }
    RGBA32f& operator -= (float f) { R-=f;  G-=f;  B-=f;  return *this; }
    RGBA32f  operator * (float f) const { return {R*f, G*f, B*f}; }
    RGBA32f  operator / (float f) const { return {R/f, G/f, B/f}; }
    RGBA32f  operator + (float f) const { return {R+f, G+f, B+f}; }
    RGBA32f  operator - (float f) const { return {R-f, G-f, B-f}; }

    void set (float r, float g, float b, float a=1.0);
    void setFromUNORM(RGBA& pix) {set(pix.R/255.f, pix.G/255.f, pix.B/255.f, pix.A/255.f);}
    RGBA32f lerp(const RGBA32f& c, float f);  // interpolate color between 2 pixels
    float Luminocity() { return R*0.3f + G*0.59f + B*0.11f; }  // toGray
    void Print() { printf("(%+6.3f,%+6.3f,%+6.3f,%+6.3f)\n", (double)R ,(double)G ,(double)B, (double)A); }

    RGBA16f  toRGBA16f() { return RGBA16f(R,G,B,A); }
    uint32_t toRGB10(){ return clampi(A*3,0,3)<<30|clampi(B*1023,0,1023)<<20|clampi(G*1023,0,1023)<<10|clampi(R*1023,0,1023);}
    uint32_t toRGB9E5();  // HDR in 32bpp: 9-bit per channel RGB with 5 bit shared exponent
    operator vec4 () { return vec4(R,G,B,A); }
};
//-------------------------------------------------------------------------------

//---------------------CImage-----------------------
class CImage : public CImageBase {
    RGBA UV_bilinear(float u, float v);
public:
    void write_pgm(const char* filename, bool sRGB = true);
    using CImageBase::CImageBase;

    CImage(){ format = R8G8BA8; }
    CImage(const char* filename){ Load(filename); }
    CImage(int width, int height, RGBA color = RGBA(0,0,0)){ SetSize(width, height); Clear(color); }
    void SetSize(int width, int height) {CImageBase::SetSize(width, height, 32); format = R8G8BA8;}
    RGBA* Buffer() const { return (RGBA*)buf; }
    RGBA& Pixel(int x, int y);
    RGBA UV(float u, float v);
    void Clear(RGBA color=RGBA(0,0,0));
    void Clear(RGBA color, bool R, bool G, bool B, bool A);
    CImage Mipmap();
    bool Load(const char* filename, bool flip = false);                       // Load from file (flip = vertically)
    bool Load_from_mem(const void* addr, int len, bool flip = false);         // Load and decompress from memory
    void Save(const char* filename);                                          // Save to file
    void Blend(CImage& other, float R_lerp, float G_lerp, float B_lerp);      // Blend with another CImage
    void sRGBtoUNORM(float brightness=0.0f, float contrast=1.0f);             // Undo jpg gamma correction. (Loses some fidelity)
    void UNORMtoSRGB();                                                       // Appy gamma correction.(2.2)(Loses some fidelity)
    void BGRAtoRGBA();                                                        // Swap red an blue channels
    void Copy(const void* addr, int bpp=4, bool Vflip=false);  //Copy&convert from 1/2/3/4 Bpp formats(external) to RGBA(local)
    operator RGBA* () const {return (RGBA*)buf;}
    operator uint* () const {return (uint*)buf;}
    RGBA& operator[](int x) {return (RGBA&)*((RGBA*)buf+x);}

    CImageBase asGrayHQ(ColorSpace cs);                      // higher quality
    CImageBase asGray() {return Pack(*this, GRAY8   ); }  // faster
    CImageBase as4444() {return Pack(*this, R4G4B4A4); }
    CImageBase as565 () {return Pack(*this, R5G6B5  ); }
    CImageBase as88  () {return Pack(*this, R8G8    ); }
    CImageBase asBC1 () {return Pack(*this, BC1     ); }
    CImageBase asBC1A() {return Pack(*this, BC1a    ); }
    CImageBase asBC3 () {return Pack(*this, BC3     ); }
    CImageBase asBC4 () {return Pack(*this, BC4     ); }
    CImageBase asBC5 () {return Pack(*this, BC5     ); }
    CImageBase asYUV () {return Pack(*this, YUV422  ); }
};
//--------------------------------------------------
//---------------------CImage32f--------------------
class CImage32f : public CImageBase {
    RGBA32f UV_bilinear(float u, float v);
public:
    using CImageBase::CImageBase;
    CImage32f(){ format = R32G32B32A32; }
    CImage32f(CImage& img);
    CImage32f(const char* filename){ Load(filename); }
    CImage32f(int width, int height){ SetSize(width, height); }
    void SetSize(int width, int height) {CImageBase::SetSize(width, height, 128); format = R32G32B32A32;}
    RGBA32f* Buffer() const { return (RGBA32f*)buf; }
    operator RGBA32f* () const {return (RGBA32f*)buf;}
    RGBA32f& operator[](int x) {return (RGBA32f&)*((RGBA32f*)buf+x);}
    RGBA32f& Pixel(int x, int y);
    RGBA32f UV(float u, float v);

    void Clear(RGBA32f color = RGBA32f(0,0,0,0));
    void Bleed(uint32_t margin = 1);  // add color-bleed-margins around texture-islands
    void Blur();
    CImage32f Mipmap();
    //void toGray();
    //vec3 GetSunVec();
    ivec2 GetBrightestPixel();
    CImage toLDR(float brightness=0.0, float contrast=1.0, float gamma=2.2f);
    bool Load(const char* filename, bool flip = false);
    void Save(const char* filename);

    CImageBase asRGB9E5()  { return Pack(*this,       RGB9E5); }
    CImageBase asRGB10()   { return Pack(*this,  A2B10G10R10); }
    CImageBase asRGBA16f() { return Pack(*this, R16G16B16A16); }
};
//--------------------------------------------------
//---------------------CCubemap---------------------
enum eCubeface{ FRONT=0, BACK=1, BOTTOM=2, TOP=3, RIGHT=4, LEFT=5 };

struct CCubemap {
    CImage32f face[6];                                                // Load cubemap from 6 cube faces
    CCubemap(){}
    CCubemap(const char* filename, MagFilter filter=LINEAR){ LoadPanorama(filename, filter); }
    ~CCubemap(){Clear();}
    void Clear() { for(auto& f:face) f.SetSize(0,0); }
    void LoadPanorama(const char* filename, MagFilter filter=LINEAR); // Load from spherical panorama image
    vec3 GetSunVec(RGBA32f* flux = 0);
};
//--------------------------------------------------

#endif
