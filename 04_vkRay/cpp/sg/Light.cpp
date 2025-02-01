#include "Light.h"

CLight::CLight(const char* name) : CObject(name) {
    type = ntLIGHT;
    if(!!default_allocator) Init();
    else LOGW("CLight:  Allocator not initialized yet. Call Light.Init when its ready.");
}

void CLight::Init() {
    light_ubo.Allocate(sizeof(LightUniform));
}

void CLight::Apply() {
    light_uniform.enabled  = enabled;
    light_uniform.color    = color;
    light_uniform.position = position;
    light_ubo.Update(&light_uniform);
}

void CLight::Transform() {
    CObject::Transform();
    Apply();
}
