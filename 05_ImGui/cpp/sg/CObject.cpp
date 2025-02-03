#include "CObject.h"
#include "matrix.h"

#ifdef WIN32
#include "windows.h"
struct UseUTF8{UseUTF8(){SetConsoleOutputCP(CP_UTF8);}}utf8;
#endif


VkCommandBuffer CObject::commandBuffer = 0;
CamUniform CObject::cam_uniform {};

//---CObject---
void CObject::Transform() {
    if(Parent()) worldMatrix = Parent()->worldMatrix * matrix;
    //else worldMatrix.Clear();
}

//Recursively call fn for all child-nodes in the tree
void CObject::recurse(recurse_fn fn) {
    fn(*this);
    CObject* child = (CObject*)FirstChild();
    while(child) {
        child->recurse(fn);
        child = (CObject*)child->Next();
    }
}

CObject* CObject::Find(const std::string& name) {
    CObject* found = 0;
    auto lamda = [&](CObject& node){ if(node.name==name) found = &node; };
    recurse(lamda);
    return found;
}

std::vector<CObject*> CObject::FindAll(NodeType type) {  // return a list of objects of given type
    std::vector<CObject*> list;
    recurse([&](CObject& node){ if(node.type==type) list.push_back(&node); } );
    return list;
}

void CObject::Visible_nodes(bool flag) {
    recurse( [&](CObject& node){ node.visible = flag; } );
}

void CObject::Transform_nodes() {
    recurse( [&](CObject& node){ node.Transform(); } );
}

void CObject::Init_nodes() {
    recurse( [&](CObject& node){ node.Init(); } );
}

void CObject::Draw_nodes(VkCommandBuffer cmd) {
    commandBuffer = cmd;
    recurse( [&](CObject& node){ node.Draw(); } );
}

//-------------------------------------------------------------------

//-------------------------------PRINT-------------------------------
void CObject::Print() {  // print scene graph tree view
    printf("%s","\n----Scene Graph:----\n");
    CObject& node = *this;
    node.recurse([&](CObject& node) {
        std::string prefix = (!!node.Next()) ? "├─" : "└─";
        CNode* item = &node;
        if(!node.Parent()) prefix = "──"; else
        while((item = item->Parent())) {
            prefix = (item->Next()) ? "│ "+prefix : "  "+prefix;
        }
        printf("%s%s : %s\n",prefix.c_str(), node.typeStr(), node.name.c_str());
    });
    printf("\n");
}
//-------------------------------------------------------------------
