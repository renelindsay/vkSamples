#ifndef COBJECT_H
#define COBJECT_H

#include <string>
#include <functional>
#include "matrix.h"
#include "CNode.h"
#include "vkray.h"

#define DOUBLE_PRECISION

#ifdef DOUBLE_PRECISION
    typedef dmat4 MAT4;
#else
    typedef mat4 MAT4;
#endif

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
    MAT4 matrix;            // Local Transform matrix, relative to parent
    MAT4 worldMatrix;       // World matrix of this object (derived from matrix)
    std::string type = "Node";
    std::string name = "";
    bool visible = true;
    int hitGroup = -1;  // 0=miss 1=hit

    static VkCommandBuffer commandBuffer;
    static CamUniform cam_uniform;

    CObject() {}
    CObject(const char* name) : name(name) {}

    virtual void Init(){}
    virtual void Transform();
    virtual void Draw(){}
    //--- RAYTRACE ---
    virtual void AddToBLAS(VKRay& rt){};
    virtual void UpdateBLAS(VKRay& rt){};
    //----------------
    virtual CObject* Parent() { return (CObject*)(_parent); }
    CObject& GetRoot() { CObject* obj = this;  while(obj->Parent()) obj = obj->Parent(); return *obj; }

    //-- Recurse --
    typedef std::function<void(CObject& curr)> recurse_fn;
    void recurse(recurse_fn fn);

    CObject* Find(std::string_view name);                  // find child node by name
    std::vector<CObject*> FindAll(std::string_view type);  // find all nodes of given type
    std::vector<CObject*> GetRenderList();            // list nodes to render
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
