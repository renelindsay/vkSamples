// Renders a cube or skybox, using a cubemap texture
// Skybox, is a cube, centered on the camera, with faces facing inward.

#ifndef CSKYBOX_H
#define CSKYBOX_H

#include "CObject.h"
#include "Material.h"
#include "CPipeline.h"

//---------------------------------------------------------

class CBox : public CObject {
protected:
    uboData ubo_data;
    VkDescriptorSets  descriptorSets;
    UBO ubo;
    VBO vbo;
    IBO ibo;
    void Bind();
public:
    CPipeline* pipeline=0;
    CvkImage*  cubemap =0;
    bool flipped = false;

    CBox(const char* name="box") : CObject(name) { type = "Box"; hitGroup = 0; }
    void Init();
    void Draw();
};

//---------------------------------------------------------

class CSkybox : public CBox {
public: 
    CSkybox(const char* name="skybox") : CBox(name) { type = "Skybox"; flipped = true; }
    void Draw();
};

//---------------------------------------------------------

#endif
