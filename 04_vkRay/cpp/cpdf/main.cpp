#ifndef CPDF_TEST_H
#define CPDF_TEST_H

#include <stdio.h>
#include "CImage.h"

#define POINT_ON_DISK
//#define POINT_ON_SPHERE

float hpi = pi/2.0f;

int main() {
    CImage32f img(512,512);
    img.Clear();
    float p = 0;
    float h = 0;
    //int cnt = 65535*8;
    int cnt = 8192*8;
    //int cnt = 16;

#ifdef POINT_ON_DISK
    repeat(cnt) {  //diffuse distribution
        float val = (i/(float)cnt);
        //printf(" %f", val);

        float r = (float)rand()/RAND_MAX;
        //printf(" %f", r);
        //p++;
        h+=1.f+r;
        //h+=1.0f+r*(1.2-val);
        //h+=1.0f+r*(1.5-sqrt(val));
        //h = val * 3.5 * 2 * pi * toDeg;

        vec3 ray(0,0,1);

        //p = acos(val) / hpi;
        //p = asin(sqrt(val));
        //p = asin(val) / pi;
        //p = val*hpi;
        //p = sqrt(val)*hpi;
        p = asin(sqrt(val));


        ray.RotateX(p*toDeg);
        ray.RotateZ(h);

        //ray.RotateX(-90);

        int x = ray.x * 255 + 255;
        int y = ray.y * 255 + 255;
        img.Pixel(x,y) += RGBA32f(0.1f,0.1f,0.1f,1);
        //img.Pixel(x,y) += RGBA32f(0.5f,0.5f,0.5f,1);
        //img.Pixel(x,y) += RGBA32f(1.0f,1.0f,1.0f,1);
    }
#endif

#ifdef POINT_ON_SPHERE
    repeat(cnt) { //sphere uniform distribution
        float rx = (float)rand()/RAND_MAX;
        //float ry = (float)rand()/RAND_MAX;
        float rz = (float)rand()/RAND_MAX;
        //vec3 ray(rx,ry,rz);

        rz = rz*2.f - 1.0;  // sphere

        float rad = rx * pi *2.f;
        float fx = cos(rad);
        float fy = sin(rad);
        float fz = sqrt(1.f-rz*rz);
        vec3 ray(fx*fz, fy*fz, rz);

        ray.RotateX(-90);
        int x = ray.x * 255 + 255;
        int y = ray.y * 255 + 255;
        img.Pixel(x,y) += RGBA32f(0.1f,0.1f,0.1f,1);
        //img.Pixel(x,y) += RGBA32f(0.5f,0.5f,0.5f,1);
        //img.Pixel(x,y) += RGBA32f(1.0f,1.0f,1.0f,1);
    }
#endif

    img.toLDR().Save("img.png");
    return 0;
}

#endif
