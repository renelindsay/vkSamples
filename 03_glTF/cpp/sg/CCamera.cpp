#include "CCamera.h"

CCamera::CCamera(const char* name) : CObject(name) {
    type = "Camera";
    SetPerspective(1, 45, 1, 1000);
    if(!!default_allocator) Init();
    else LOGW("CCamera:  Allocator not initialized yet. Call CCamera.Init() when its ready.");
}

void CCamera::Init() {
    cam_ubo.Allocate(sizeof(CamUniform));
}

// Use the camera's WorldMatrix, to calculate the ViewMatrix.
void CCamera::Apply(VkFence fence) {
    cam_uniform.proj = proj_matrix;
    cam_uniform.view = worldMatrix.WorldToView();
    cam_uniform.projInverse = cam_uniform.proj.Inverse();
    cam_uniform.viewInverse = cam_uniform.view.Inverse_fast();
    cam_uniform.flags = flags;
    cam_ubo.fence=fence;
    cam_ubo.Update(&cam_uniform);
}

void CCamera::SetPerspective(float aspect, float fovy, float nearplane, float farplane) {
    proj_matrix.SetPerspective(aspect, fovy, nearplane, farplane);
 }

void CCamera::SetOrthographic(float left, float right, float bottom, float top, float nearplane, float farplane) {
    proj_matrix.SetOrtho(left, right, bottom, top, nearplane, farplane);
}

void CCamera::Transform() {
    if(orbit_radius) matrix.position() = matrix.zAxis() * orbit_radius;
    CObject::Transform();
    //Apply();
}

void CCamera::Orbit(double angle, double pitch) {
    vec3 axis(matrix.m10, matrix.m11, matrix.m12);
    matrix.Rotate(angle, axis);
    matrix.RotateX(pitch);
    //matrix.position() = matrix.zAxis() * orbit_radius;
}
