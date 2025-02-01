#ifndef COBJECT_H
#define COBJECT_H

#include <string>
#include <functional>
#include "matrix.h"
#include "CNode.h"
#include "Buffers.h"
#include "CPipeline.h"

static const char* ntStr[] { "Node", "Camera", "Light", "Mesh", "Quad", "Cube", "Sphere", "Shape", "glTF", "Box", "Skybox", "DEM"};
             enum NodeType { ntNODE, ntCAMERA, ntLIGHT, ntMESH, ntQUAD, ntCUBE, ntSPHERE, ntSHAPE, ntGLTF, ntBOX, ntSKYBOX, ntDEM};

struct CamUniform {
    mat4 view;
    mat4 proj;
    mat4 viewInverse;
    mat4 projInverse;
    uint flags;
};

class CObject : public CNode {
public:
    mat4 worldMatrix;             // World matrix of this object (derived from matrix)
    NodeType type = ntNODE;
    std::string name = "";
    const char* typeStr() { return ntStr[type]; }
    bool visible = true;
    int hitGroup = -1;  // 0=miss 1=hit

    static VkCommandBuffer commandBuffer;
    static CamUniform cam_uniform;

    CObject() {}
    CObject(const char* name) : name(name) {}
    mat4 matrix;                  // Transform matrix, relative to parent

    virtual void Init(){}
    virtual void Bind(){}
    virtual void Transform();
    virtual void Draw(){}

    virtual CObject* Parent() { return (CObject*)(_parent); }
    CObject& GetRoot() { CObject* obj = this;  while(obj->Parent()) obj = obj->Parent(); return *obj; }

    //-- Recurse --
    typedef std::function<void(CObject& curr)> recurse_fn;
    void recurse(recurse_fn fn);

    CObject* Find(const std::string& name);        // find child node by name
    std::vector<CObject*> FindAll(NodeType type);  // find all nodes of given type
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
