#ifndef CCAMERA_H
#define CCAMERA_H

#include "CObject.h"
/*
struct CamUniform {
    mat4 view;
    mat4 proj;
    mat4 viewInverse;
    mat4 projInverse;
    uint flags;
};
*/
class CCamera : public CObject {
    //CamUniform cam_uniform;
  public:
    UBO cam_ubo;

#ifdef DOUBLE_PRECISION
    dmat4 proj_matrix;
#else
    mat4 proj_matrix;
#endif

    uint flags=0;

    CCamera(const char* name="camera");
    void Init();  //Allocate UBO
    void Apply(VkFence fence=nullptr);
    void SetPerspective(float aspect, float fovy, float nearplane, float farplane);
    void SetOrthographic(float left, float right, float bottom, float top, float nearplane, float farplane);

    float FovY() const { return (float)(-(atan(1.0 / proj_matrix.m11) * 2.0) * toDEG);}
    float Near() const { return proj_matrix.m23 / (proj_matrix.m22 - 1.0); }
    float Far () const { return proj_matrix.m23 / (proj_matrix.m22 + 1.0); }

    void Transform();
    //void Draw(){};
};

#endif
