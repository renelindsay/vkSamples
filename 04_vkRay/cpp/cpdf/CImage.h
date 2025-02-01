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
    uint8_t R=0; uint8_t G=0; uint8_t B=0;
};
//-------------------------------------------------------------------------------

//-------------------------------------------------------------------------------
// 8 bit per channel sRGBA (gamma or linear)
struct RGBA { 
    uint8_t R; uint8_t G; uint8_t B; uint8_t A; 
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
};
//-------------------------------------------------------------------------------

//-------------------------------------------------------------------------------
// 32bit per channel float (linear)
struct RGBA32f { 
    float R; float G; float B; float A; 
    RGBA32f();
    RGBA32f(float r, float g, float b, float a=1);
    RGBA32f(RGBA pix);                   // converts sRGB to linear
    RGBA32f(vec4 col);
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

    operator vec4 () { return vec4(R,G,B,A); }
};
//-------------------------------------------------------------------------------

enum MagFilter {NEAREST, LINEAR};
enum WrapMode  {wmCLAMP, wmREPEAT};
enum ColorSpace{csSRGB, csUNORM};
enum ImgFormat { NONE=0, GRAY8=1, GRAY16=2, R8G8B8=3, R8G8BA8=4, R4G4B4A4=5,
                 R5G6B5=6, BC1=7, BC1a=8, BC3=9, BC4=10, BC5=11, R8G8=12};

//---------------------CImageBase-------------------
//class alignas(32) CImageBase {
class CImageBase {
    MOVE_SEMANTICS(CImageBase)
protected:
    int   width  = 0;
    int   height = 0;
    size_t size  = 0;
    void* buf  = nullptr;

public:
    MagFilter  magFilter  = LINEAR;
    WrapMode   wrapMode_U = wmCLAMP;
    WrapMode   wrapMode_V = wmCLAMP;
    ColorSpace colorspace = csSRGB;
    ImgFormat  format     = NONE;

    void SetSize(uint w, uint h, uint bitspp);
    size_t Size() const { return size; }  // returns buffer size in bytes
    uint  Width() const { return width; }
    uint  Height()const { return height; }
    void* Buffer()const { return buf; }
    void* Pixel(int x, int y);

    void Clear();
    CImageBase(){}
    CImageBase(uint w, uint h, ImgFormat fmt);
    ~CImageBase(){ if(!!buf) free(buf); buf=nullptr; size=0;}
    operator void* () const {return buf;}
};
//--------------------------------------------------
//---------------------CConvert---------------------
class CImage;
class Convert : public CImageBase {
    struct block{uint w,h,s;}b;  //w=xblocks h=yblocks s=bytes_per_block
    void BlockSize(const CImageBase& img, uint bitspp, uint blocksize=1);
    void* Block(int x, int y);
    void toGray(CImage& img);                    //  8bpp grayscale
    void to4444(CImage& img);                    // 16bpp color+alpha
    void to565 (CImage& img);                    // 16bpp color
    void to88  (CImage& img);                    // 16bpp 2-channel
    void toBC1 (CImage& img, bool alpha=true);   //  4bpp color       (COMPRESSED) (optional punch-through alpha)
    void toDXT (CImage& img, bool alpha=true);   //  8bpp color+alpha (COMPRESSED)
    void toBC4 (const CImageBase& grayimg);      //  4bpp grayscale   (COMPRESSED)
    void toBC5 (const CImageBase& rg_img);       //  8bpp 2-channel   (COMPRESSED)
public:
    Convert(CImage& img, ImgFormat dstFormat);
    Convert(CImageBase& img, ImgFormat dstFormat);
};
//--------------------------------------------------
//---------------------CImage-----------------------
class CImage : public CImageBase {
    RGBA UV_bilinear(float u, float v);
public:
    void write_pgm(const char* filename, bool sRGB = true);
    using CImageBase::CImageBase;

    CImage(){}
    CImage(const char* filename){ Load(filename); }
    CImage(int width, int height, RGBA color = RGBA(0,0,0)){ SetSize(width, height); Clear(color); }
    void SetSize(int width, int height) {CImageBase::SetSize(width, height, 32);}
    RGBA* Buffer() const { return (RGBA*)buf; }
    RGBA& Pixel(int x, int y);
    RGBA UV(float u, float v);
    void Clear(RGBA color=RGBA(0,0,0));
    void Clear(RGBA color, bool R, bool G, bool B, bool A);

    bool Load(const char* filename, bool flip = false);                       // Load from file (flip = vertically)
    bool Load_from_mem(const void* addr, int len, bool flip = false);         // Load and decompress from memory
    void Save(const char* filename);                                          // Save to file
    void Blend(CImage& other, float R_lerp, float G_lerp, float B_lerp);      // Blend with another CImage
    void sRGBtoUNORM(float brightness=0.0f, float contrast=1.0f);             // Undo jpg gamma correction. (Loses some fidelity)
    void UNORMtoSRGB();                                                       // Appy gamma correction.(2.2)(Loses some fidelity)
    void BGRAtoRGBA();                                                        // Swap red an blue channels
    void Copy(const void* addr, int bpp=4, bool Vflip=false);  //Copy&convert from 1/2/3/4 Bpp formats(external) to RGBA(local)
    //operator vec4 () { return *(vec4*)this; }
    operator RGBA* () const {return (RGBA*)buf;}

    CImageBase asGray(ColorSpace cs);                        // higher quality
    CImageBase asGray() {return Convert(*this, GRAY8   ); }  // faster
    CImageBase as4444() {return Convert(*this, R4G4B4A4); }
    CImageBase as565 () {return Convert(*this, R5G6B5  ); }
    CImageBase as88  () {return Convert(*this, R8G8    ); }
    CImageBase asBC1 () {return Convert(*this, BC1     ); }
    CImageBase asBC1A() {return Convert(*this, BC1a    ); }
    CImageBase asBC3 () {return Convert(*this, BC3     ); }
    CImageBase asBC4 () {return Convert(*this, BC4     ); }
    CImageBase asBC5 () {return Convert(*this, BC5     ); }
};
//--------------------------------------------------
//---------------------CImage32f--------------------
class CImage32f : public CImageBase {
    RGBA32f UV_bilinear(float u, float v);
public:
    using CImageBase::CImageBase;
    CImage32f(){}
    CImage32f(CImage& img);
    CImage32f(const char* filename){ Load(filename); }
    CImage32f(int width, int height){ SetSize(width, height); }
    void SetSize(int width, int height) {CImageBase::SetSize(width, height, 128);}
    RGBA32f* Buffer() const { return (RGBA32f*)buf; }

    RGBA32f& Pixel(int x, int y);
    RGBA32f UV(float u, float v);

    void Clear(RGBA32f color = RGBA32f(0,0,0,0));
    void Bleed(uint32_t margin = 1);  // add color-bleed-margins around texture-islands
    void Blur();
    //void toGray();
    //vec3 GetSunVec();
    CImage toLDR(float brightness=0.0, float contrast=1.0, float gamma=2.2f);
    bool Load(const char* filename, bool flip = false);
    void Save(const char* filename);
};
//--------------------------------------------------
//---------------------CCubemap---------------------
enum eCubeface{ FRONT=0, BACK=1, BOTTOM=2, TOP=3, RIGHT=4, LEFT=5 };

struct CCubemap {
    CImage face[6];
    void Clear() { for(auto& f:face) f.SetSize(0,0); }
};
//--------------------------------------------------

#endif
