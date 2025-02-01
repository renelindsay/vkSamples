#ifndef DISPMAP_H
#define DISPMAP_H

#include "Mesh.h"

//-----------------------------DEM----------------------------
class DEM : public CMesh {
public:
    uint width  = 0;
    uint height = 0;

    DEM(const char* name="dem") : CMesh(name) {type = ntDEM; }
    void Init(){};
    void Build(CImage& img, float zscale=1);
};
//------------------------------------------------------------

#endif
