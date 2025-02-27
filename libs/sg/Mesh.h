#ifndef CMESH_H
#define CMESH_H

#include <vector>
#include "CShader.h"
#include "CObject.h"
#include "Material.h"
#include "CPipeline.h"

//struct Vertex {vec3 pos; vec3 nrm; vec2 tc;};
//----------------------------MESH----------------------------
class CMesh : public CObject {
    uboData ubo_data;
    VkDescriptorSets  descriptorSets;
    void Bind();
    void UpdateUBO();
    int blasInx = -1;

public:
    UBO ubo;
    VBO vbo;
    IBO ibo;
    CPipeline* pipeline= 0;
    CvkImage*  cubemap = 0;
    Material   material;

    CMesh(const char* name="mesh") : CObject(name) { type = "Mesh"; hitGroup = 1; }
    void Init();
    void Draw();

    //--- RAYTRACE ---
    void AddToBLAS (VKRay& rt);
    void UpdateBLAS(VKRay& rt);
    //----------------
};
//------------------------------------------------------------
//----------------------------QUAD----------------------------
class CQuad : public CMesh {
public:
    CQuad(const char* name="quad") : CMesh(name) {type = "Quad"; }
    void Init();
};
//------------------------------------------------------------
//----------------------------CUBE----------------------------
class CCube : public CMesh {
public:
    CCube(const char* name="cube") : CMesh(name) { type = "Cube"; }
    void Init();
};
//------------------------------------------------------------
//---------------------------SPHERE---------------------------
class CSphere : public CMesh {
protected:
    float radius    = 1;
    uint  slices    = 64;
    uint  stacks    = 32;
    float slicesAng = 360;
    float stacksAng = 180;
    bool  invert    = false;
    void Build(float Radius=1.0, uint Slices=64, uint Stacks=32, float SlicesAng=360, float StacksAng=180, bool Invert = false);
public:
    CSphere(const char* name="sphere") : CMesh(name) { type = "Sphere"; }
    void Params(float Radius=1.0, uint Slices=64, uint Stacks=32, float SlicesAng=360, float StacksAng=180, bool Invert = false);
    void Init();
};
//------------------------------------------------------------
//---------------------------SHAPE----------------------------
class CShape : public CMesh {
    void Build(std::vector<vec2>& pairs, uint Slices=64, bool flatX=true, bool flatY=true);
    uint slices = 64;
    bool flatX  = false;  // flat vs smooth normals
    bool flatY  = false;  // flat vs smooth normals
public:
    float degrees = 360.f;

    enum Shape{CONE, PIPE, CYLINDER, ARROW, RING, FAN} shape = CONE;  // TODO: CUSTOM
    CShape(const char* name="shape") : CMesh(name) { type = "Shape"; }
    void Geometry(Shape shape, uint slices=64, bool flatX=true, bool flatY=true);
    void Init();
};
//------------------------------------------------------------


#endif
