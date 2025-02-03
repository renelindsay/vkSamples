#ifndef MATERIAL_H
#define MATERIAL_H

#include <string>
#include "CShader.h"

//---- CPU side material ----
struct Material {
    std::string name;
    struct {
        vec4 albedo   = {1,1,1,1};
        vec4 emission = {0,0,0,0};
        vec4 normal   = {1,1,1,1};  // XY-scale (not used?)
        vec4 orm      = {1,1,1,1};
        //vec4& operator[](int i){return ((vec4*)this)[i];};
    } color;

    struct {
        CvkImage* albedo   = 0;
        CvkImage* emission = 0;
        CvkImage* normal   = 0;
        CvkImage* orm      = 0;    // (R=AO, G=Roughness, B=Metalness)
        //CvkImage*& operator[](int i){return ((CvkImage**)this)[i];};
    } texture;

    void Bind(CShader& shader);   // not used
};
//---------------------------

//---- GPU side material ----
struct uboData {                  // vkMaterial
    mat4 matrix;                  // world matrix
    vec4 color[4] {};             // material colors
    int  texid[4] {-1,-1,-1,-1};  // texture id's (RT only)
};
//---------------------------

#endif

