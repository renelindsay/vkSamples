// Renders a cube or skybox, using a cubemap texture
// Skybox, is a cube, centered on the camera, with faces facing inward.

#ifndef CSKYBOX_H
#define CSKYBOX_H

#include "CObject.h"
#include "Material.h"

//---------------------------------------------------------

class CBox : public CObject {
protected:
    uboData ubo_data;
    VkDescriptorSets  descriptorSets;
    UBO ubo;
    VBO vbo;
    IBO ibo;

public:
    CPipeline* pipeline=0;
    CvkImage*  cubemap =0;
    bool flipped = false;

    CBox(const char* name="box") : CObject(name) { type = ntBOX; hitGroup = 0; }
    void Init();
    void Bind();
    void Draw();
};

//---------------------------------------------------------

class CSkybox : public CBox {
public: 
    CSkybox(const char* name="skybox") : CBox(name) { type = ntSKYBOX; flipped = true; }
    void Draw();
};

//---------------------------------------------------------

#endif
