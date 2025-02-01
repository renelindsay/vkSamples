#ifndef COBJECT_H
#define COBJECT_H

#include <string>
#include <functional>
#include "matrix.h"
#include "CNode.h"
#include "Buffers.h"
#include "CPipeline.h"
#include "vkray.h"

#define DOUBLE_PRECISION

static const char* ntStr[] { "Node", "Camera", "Light", "Mesh", "Quad", "Cube", "Sphere", "Shape", "glTF", "Box", "Skybox", "DEM"};
             enum NodeType { ntNODE, ntCAMERA, ntLIGHT, ntMESH, ntQUAD, ntCUBE, ntSPHERE, ntSHAPE, ntGLTF, ntBOX, ntSKYBOX, ntDEM};

struct CamUniform {
    mat4 view;
    mat4 proj;
    mat4 viewInverse;
    mat4 projInverse;
    uint flags;
    int  sky = -1;
    uint random;
};

class CObject : public CNode {
public:
#ifdef DOUBLE_PRECISION
    dmat4 matrix;
    dmat4 worldMatrix;
#else
    mat4 matrix;
    mat4 worldMatrix;
#endif

    NodeType type = ntNODE;
    std::string name = "";
    const char* typeStr() { return ntStr[type]; }
    bool visible = true;
    int hitGroup = -1;  // 0=miss 1=hit

    static VkCommandBuffer commandBuffer;
    static CamUniform cam_uniform;

    CObject() {}
    CObject(const char* name) : name(name) {}

    virtual void Init(){};
    virtual void Bind(){};
    virtual void Transform();
    virtual void Draw(){};

    virtual void AddToBLAS(VKRay& rt){};
    virtual void UpdateBLAS(VKRay& rt){};

    virtual CObject* Parent() { return (CObject*)(_parent); }
    CObject& GetRoot() { CObject* obj = this;  while(obj->Parent()) obj = obj->Parent(); return *obj; }

    //-- Recurse --
    typedef std::function<void(CObject& curr)> recurse_fn;
    void recurse(recurse_fn fn);

    CObject* Find(const std::string& name);        // find child node by name
    std::vector<CObject*> FindAll(NodeType type);  // find all nodes of given type
    std::vector<CObject*> GetRenderList();
    //-------------

    //-- Recursive node functions --
    void Visible_nodes(bool flag);                 // show/hide all nodes in branch
    void Transform_nodes();                        // transform all nodes in branch
    void Init_nodes();
    void Draw_nodes(VkCommandBuffer cmd);
    void Print();
    //------------------------------
};

#endif
