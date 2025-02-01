#extension GL_EXT_nonuniform_qualifier : enable

#define NORMAL_PACK

struct Vertex {
    vec3 pos;
    vec3 nrm;
    vec2 tc;
};

struct VertPack {  // only use 4-byte types, to prevent realignment
    float posx;
    float posy;
    float posz;
    float norm;
    float tc0;
    float tc1;
};

#ifndef NORMAL_PACK
layout(binding = 4, set = 0) buffer  Vertices{ vec4 v[];     } vrtSets[];  // vbo
#else
layout(binding = 4, set = 0) buffer  Vertices{ VertPack v[]; } vrtSets[];  // vbo (packed)
#endif
layout(binding = 5, set = 0) buffer  Indices { uint i[];     } inxSets[];  // ibo


#ifndef NORMAL_PACK
// No pack (8 DWORD)
Vertex unpackVertex(uint index) {
    uint vrtSize = 2;  // Number of vec4 values used to represent a vertex
    vec4 d0 = vrtSets[gl_InstanceID].v[vrtSize * index + 0];
    vec4 d1 = vrtSets[gl_InstanceID].v[vrtSize * index + 1];
    Vertex v;
    v.pos = d0.xyz;
    v.nrm = vec3(d0.w, d1.x, d1.y);
    v.tc  = vec2(d1.z, d1.w);
    return v;
}

#else

// pack (6 DWORD)
Vertex unpackVertex(uint index) {  // with pack32 normal
    VertPack p = vrtSets[gl_InstanceID].v[index];
    Vertex v;
    v.pos = vec3(p.posx, p.posy, p.posz);
    int npack = floatBitsToInt(p.norm);
    v.nrm.x = float((npack<< 2)&0xFFC00000);
    v.nrm.y = float((npack<<12)&0xFFC00000);
    v.nrm.z = float((npack<<22)&0xFFC00000);   
    normalize(v.nrm);
    v.tc  = vec2(p.tc0, p.tc1);
    return v;
}
#endif

Vertex[3] getTriangle() {
    Vertex v[3];
    uint primOffs = gl_PrimitiveID * 3;
    v[0] = unpackVertex(inxSets[gl_InstanceID].i[primOffs + 0]);
    v[1] = unpackVertex(inxSets[gl_InstanceID].i[primOffs + 1]);
    v[2] = unpackVertex(inxSets[gl_InstanceID].i[primOffs + 2]);
    return v;
}



