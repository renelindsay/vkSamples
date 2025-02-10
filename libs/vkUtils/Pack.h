#ifndef PACK_H
#define PACK_H

//#include "CImage.h"

//#include <stdint.h>
#include <stdio.h>
#include <malloc.h>
#include "matrix.h"
//#include "fp16.h"

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

enum MagFilter {NEAREST, LINEAR};
enum WrapMode  {wmCLAMP, wmREPEAT};
enum ColorSpace{csSRGB, csUNORM};
enum ImgFormat { NONE=0, GRAY8=1, GRAY16=2, R8G8B8=3, R8G8BA8=4, R4G4B4A4=5,
                 R5G6B5=6, BC1=7, BC1a=8, BC3=9, BC4=10, BC5=11, R8G8=12,
                 A2B10G10R10=13, R16G16B16A16=14, R32G32B32A32=15,
                 YUV422=16, RGB9E5=17
               };

//---------------------CImageBase-------------------
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

//-----------------------Pack-----------------------
class CImageBase;
class CImage;
class CImage32f;
class Pack : public CImageBase {
    struct block{uint w,h,s;}b;  //w=xblocks h=yblocks s=bytes_per_block
    void BlockSize(const CImageBase& img, uint bitspp, uint blocksize=1);
    void* Block(int x, int y);
public:
    void toGray(CImage& img);                    //  8bpp grayscale
    void to4444(CImage& img);                    // 16bpp color+alpha
    void to565 (CImage& img);                    // 16bpp color
    void to88  (CImage& img);                    // 16bpp 2-channel
    void toBC1 (CImage& img, bool alpha=true);   //  4bpp color       (COMPRESSED) (optional punch-through alpha)
    void toBC3 (CImage& img, bool alpha=true);   //  8bpp color+alpha (COMPRESSED) (DXT)
    void toBC4 (CImage& img);                    //  4bpp grayscale   (COMPRESSED)
    void toBC5 (CImage& img);                    //  8bpp 2-channel   (COMPRESSED)
    void toYUV (CImage& img);                    // 16bpp YCbCr G8B8G8R8 (YUV422)
    void toRGB10  (CImage32f& img);              // 32bpp A2R10G10B10
    void toRGBA16f(CImage32f& img);              // 64bpp R16G16B16A16
    void toRGB9E5 (CImage32f& img);              // 32bpp HDR

public:
    Pack(CImage&     img, ImgFormat dstFormat);
    Pack(CImage32f&  img, ImgFormat dstFormat);
};
//--------------------------------------------------

#endif
