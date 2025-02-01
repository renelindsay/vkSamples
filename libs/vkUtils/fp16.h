//----------------fp16---------------
// Rene Lindsay (2023)
// Follows the IEEE 754 Half perecision floating point standard.
// Converts between single and half precision floats.
// Clamp overflows either to max value, or infinity.
//-----------------------------------

#pragma once
#ifndef FP16_H
#define FP16_H

#include <stdio.h>
#include <stdint.h>
#include "matrix.h"

// ---PICK ONE OF:---
//#define QUICK_AND_DIRTY  // Fails for 0, fractions smaller than +-0.000031 or values larger than +-65504.       (fast & buggy)
#define USE_SUBNORMALS   // Clamps overflows, and correctly handles subnormal fractions smaller than +-0.000031.  (slow & accurate)
//-------------------

//#define USE_INFINITY     // Set overflow values (>+-65504) to infinity. (Don't use this on GPU textures.)
#ifdef USE_INFINITY
#define MAXVAL 0x7C00    //inf
#else
#define MAXVAL 0x7BFF    // 65504
#endif

//----------------fp16---------------
#ifdef QUICK_AND_DIRTY
struct fp16 {
    uint16_t h;
    fp16(float f) {
        uint x = *((uint*)&f);
        h = ((x>>16)&0x8000)|((((x&0x7f800000)-0x38000000)>>13)&0x7c00)|((x>>13)&0x03ff);
    }
    operator float () const { uint x=((h&0x8000)<<16)|(((h&0x7C00)+0x1C000)<<13)|((h&0x03FF)<<13); return *(float*)&x; }
};
#endif
//-----------------------------------

//-----------------------------------
#ifdef USE_SUBNORMALS
struct fp16 { 
    uint16_t h;
    fp16() : h() {};
    fp16(float f) {
        uint x = *((uint*)&f);
        uint s = (x>>16)&0x8000;
        int  e = (((x>>23)&0xFF)-127+15);
        if(e==0x1F) { h=s|MAXVAL; return;}             // clamp overflow
        if(e>0) h = s|e<<10|(x>>13)&0x03ff;            // normals
        else    h = s|0|(0x200|((x>>14)&0x01ff))>>-e;  // subnormals
    }

    operator float () const {
        uint s = (h&0x8000)<<16;
        uint e = (h&0x7C00);
        uint m = h&0x03FF;
        if(e) { e = (e+0x1C000)<<13; }
        else {  // subnormal mode
            int c = 10;
            if(m&0x3C0) { m >>= 6; c = 4;}  // shortcut
            while(m>>=1) --c;
            m = ((h<<c)&0x03FF);
            e = ((e>>10)+1+112-c)<<23;
        }
        uint x = s|e|m<<13;
        return *(float*)&x;
    }
};
#endif
//-----------------------------------

//------------Unit Tests-------------
//#define TEST_HALF
#ifdef  TEST_HALF
static void PrintHalf(fp16 h) {
    for(int i=15; i>=0; i--){
        printf("%d",(h.h>>i&1));
        if(i==15||i==10) printf(" ");
    }
    printf("\n");
};

static void PrintFloat(float f) {
    for(int i=31; i>=0; i--){
        printf("%d",((*(uint32_t*)&f)>>i&1));
        if(i==31||i==23) printf(" ");
    }
    printf(" : %f\n",f);
};

static void TestHalf(float f) {  //Convert float to half, and back, and print bit patterns
    fp16 h = f;
    PrintFloat(f);
    PrintHalf (h);
    f=h;
    PrintFloat(f);
    printf("\n");
}

static void RunHalfTest() {
    //TestHalf(0);
    //TestHalf(3.14159265);  // normal value
    TestHalf(0.00000173);  // subnormal value
    //TestHalf(66535);       // overflow
    TestHalf(0.0000043);
    TestHalf(0.00001193);
}
#endif
//-----------------------------------

#endif
