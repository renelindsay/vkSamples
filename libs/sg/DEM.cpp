#include "DEM.h"

#undef repeat
#undef forXY
#define repeat(COUNT) for(uint32_t i = 0; i < (COUNT); ++i)
#define forXY(X, Y) for(uint y = 0; y < Y; ++y) for(uint x = 0; x < X; ++x)


void DEM::Build(CImage& img, float zscale) {
    uint w = width = img.Width();
    uint h = height = img.Height();
    uint64_t vrt_cnt = w * h;
    VertsArray vertices(vrt_cnt);

    //Timer t;
    forXY(w, h) {
        uint inx = x + (y * w);
        float z=img.Pixel(x,y).R;
        Vertex& vert = vertices[inx];
        vert.pos = vec3(x, y, z*zscale) / w;
        vert.tc  = vec2(x/(w-1.f), y/(h-1.f));
    }

    // Index Array
    std::vector<uint> indices;
    auto AddTriangle = [&](uint v0, uint v1, uint v2) {
        indices.push_back(v0);
        indices.push_back(v1);
        indices.push_back(v2);
    };
    auto AddQuad = [&](uint v0, uint v1, uint v2, uint v3) {
        AddTriangle(v0, v1, v2);
        AddTriangle(v1, v3, v2);
    };
    //t.Print(" Vertex array");

    forXY(w-1, h-1) {
        uint pt0 = (x+0)%w + ((y+0)*w);
        uint pt1 = (x+1)%w + ((y+0)*w);
        uint pt2 = (x+0)%w + ((y+1)*w);
        uint pt3 = (x+1)%w + ((y+1)*w);
        AddQuad(pt0, pt1, pt2, pt3);
    }
    //t.Print(" Index array ");

    //LOGI("Generate: Normals...     ");
    // Generate Normals
    uint face_count = (uint)(indices.size()/3);
    repeat(face_count) {
        Vertex* vt0 = &vertices[indices[i*3+0]];
        Vertex* vt1 = &vertices[indices[i*3+1]];
        Vertex* vt2 = &vertices[indices[i*3+2]];
        vec3 v01 = vt1->pos - vt0->pos;
        vec3 v02 = vt2->pos - vt0->pos;
        vec3 cross = v01.cross(v02);
        float len = cross.length();
        int odd = i&1; // odd+even triangle per quad
        if(len>0) {    // prevent nan
            vec3 face_normal = cross / len;
            vt0->nrm += face_normal / (1.f+odd);
            vt1->nrm += face_normal /  2.f;
            vt2->nrm += face_normal / (2.f-odd);
        }
    }
    vbo.Data(vertices);  //pack
    ibo.Data(indices);
}
