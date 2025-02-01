#ifndef LIGHT_H
#define LIGHT_H

#include "CObject.h"
//#include "Mesh.h"


// light:
//   position
//   normal
//   radiance
//   solid_angle
//


struct LightUniform {
    bool enabled=false;
    char pad[3]{};
    vec4 color;
    vec4 position;  // or direction, if w=0  (WIP)
};

class CLight: public CObject {
//public:
    LightUniform light_uniform;    
public:
    UBO light_ubo;

    bool enabled = false;
    RGBA32f color {2,2,2,1};
    vec4 position {0,1,0,0};

  CLight(const char* name="light");
  ~CLight(){};
  void Init();
  void Apply();
  void Transform();
  //void Draw();
};

#endif
