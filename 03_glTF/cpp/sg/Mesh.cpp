#include "Mesh.h"

#undef repeat
#undef forXY
#define repeat(COUNT) for(uint32_t i = 0; i < (COUNT); ++i)
#define forXY(X, Y) for(uint32_t y = 0; y < Y; ++y) for(uint32_t x = 0; x < X; ++x)

//--------------------------CMesh-----------------------------
void CMesh::Init() {}

void CMesh::Bind() {
    assert(pipeline && "Pipeline not set.");
    CShader& shader = pipeline->shader;
    shader.Bind("model", ubo);
    //material.Bind(shader);
    shader.Bind("tex_albedo",   material.texture.albedo);
    shader.Bind("tex_emission", material.texture.emission);
    shader.Bind("tex_normal",   material.texture.normal);
    shader.Bind("tex_orm",      material.texture.orm);
    if(cubemap) shader.Bind("tex_cubemap", cubemap);
    shader.UpdateDescriptorSets(descriptorSets);
}

void CMesh::UpdateUBO() {  // (does NOT update textures)
    ubo_data.matrix   = worldMatrix;
    ubo_data.color[0] = material.color.albedo;
    ubo_data.color[1] = material.color.emission;
    ubo_data.color[2] = material.color.normal;
    ubo_data.color[3] = material.color.orm;
    ubo.Set(&ubo_data, sizeof(ubo_data));
}

void CMesh::Draw() {
    if(!visible)return;
    UpdateUBO();
    Bind();
    pipeline->Bind(commandBuffer, descriptorSets);

    // Geometry
    vkCmdBindVertexBuffer(commandBuffer, vbo);
    vkCmdBindIndexBuffer (commandBuffer, ibo, 0, VK_INDEX_TYPE_UINT32);
    vkCmdDrawIndexed     (commandBuffer, ibo.Count(), 1, 0, 0, 0);
}
//------------------------------------------------------------

//----------------------------QUAD----------------------------
void CQuad::Init() {
    static const Vertex verts[] = {  // {{vrt}, {norm}, {tc}}
        {{-0.5f,-0.5f, 0.0f}, { 0.0f, 0.0f, 1.0f}, {0.0f, 1.0f}},
        {{ 0.5f,-0.5f, 0.0f}, { 0.0f, 0.0f, 1.0f}, {1.0f, 1.0f}},
        {{ 0.5f, 0.5f, 0.0f}, { 0.0f, 0.0f, 1.0f}, {1.0f, 0.0f}},
        {{-0.5f, 0.5f, 0.0f}, { 0.0f, 0.0f, 1.0f}, {0.0f, 0.0f}},
    };
    static const uint32_t indices[] = {0,1,2, 2,3,0}; // 2 triangles

    uint32_t stride = sizeof (Vertex);          // size of each vertex in bytes
    uint32_t count  = sizeof (verts) / stride;  // number of vertexes in the array
    vbo.Data(verts, count);                     // packs normals
    ibo.Data(indices, 6);
}
//------------------------------------------------------------

//----------------------------CUBE----------------------------
void CCube::Init() {
    const std::vector<Vertex> verts = {  // {{vrt}, {norm}, {tc}}
        //front
        {{-0.5f,-0.5f, 0.5f}, { 0.0f, 0.0f, 1.0f}, {0.0f, 1.0f}},
        {{ 0.5f,-0.5f, 0.5f}, { 0.0f, 0.0f, 1.0f}, {1.0f, 1.0f}},
        {{ 0.5f, 0.5f, 0.5f}, { 0.0f, 0.0f, 1.0f}, {1.0f, 0.0f}},
        {{-0.5f, 0.5f, 0.5f}, { 0.0f, 0.0f, 1.0f}, {0.0f, 0.0f}},
        //back
        {{ 0.5f,-0.5f,-0.5f}, { 0.0f, 0.0f,-1.0f}, {0.0f, 1.0f}},
        {{-0.5f,-0.5f,-0.5f}, { 0.0f, 0.0f,-1.0f}, {1.0f, 1.0f}},
        {{-0.5f, 0.5f,-0.5f}, { 0.0f, 0.0f,-1.0f}, {1.0f, 0.0f}},
        {{ 0.5f, 0.5f,-0.5f}, { 0.0f, 0.0f,-1.0f}, {0.0f, 0.0f}},
        //left
        {{-0.5f,-0.5f,-0.5f}, {-1.0f, 0.0f, 0.0f}, {0.0f, 1.0f}},
        {{-0.5f,-0.5f, 0.5f}, {-1.0f, 0.0f, 0.0f}, {1.0f, 1.0f}},
        {{-0.5f, 0.5f, 0.5f}, {-1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}},
        {{-0.5f, 0.5f,-0.5f}, {-1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}},
        //right
        {{ 0.5f,-0.5f, 0.5f}, { 1.0f, 0.0f, 0.0f}, {0.0f, 1.0f}},
        {{ 0.5f,-0.5f,-0.5f}, { 1.0f, 0.0f, 0.0f}, {1.0f, 1.0f}},
        {{ 0.5f, 0.5f,-0.5f}, { 1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}},
        {{ 0.5f, 0.5f, 0.5f}, { 1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}},
        //top
        {{-0.5f,-0.5f,-0.5f}, { 0.0f,-1.0f, 0.0f}, {0.0f, 1.0f}},
        {{ 0.5f,-0.5f,-0.5f}, { 0.0f,-1.0f, 0.0f}, {1.0f, 1.0f}},
        {{ 0.5f,-0.5f, 0.5f}, { 0.0f,-1.0f, 0.0f}, {1.0f, 0.0f}},
        {{-0.5f,-0.5f, 0.5f}, { 0.0f,-1.0f, 0.0f}, {0.0f, 0.0f}},
        //bottom
        {{-0.5f, 0.5f, 0.5f}, { 0.0f, 1.0f, 0.0f}, {0.0f, 1.0f}},
        {{ 0.5f, 0.5f, 0.5f}, { 0.0f, 1.0f, 0.0f}, {1.0f, 1.0f}},
        {{ 0.5f, 0.5f,-0.5f}, { 0.0f, 1.0f, 0.0f}, {1.0f, 0.0f}},
        {{-0.5f, 0.5f,-0.5f}, { 0.0f, 1.0f, 0.0f}, {0.0f, 0.0f}},
    };

    const std::vector<uint32_t> index = {
        0, 1, 2,  2, 3, 0,
        4, 5, 6,  6, 7, 4,
        8, 9,10, 10,11, 8,
       12,13,14, 14,15,12,
       16,17,18, 18,19,16,
       20,21,22, 22,23,20
    };

    vbo.Data(verts);
    ibo.Data(index);
}
//------------------------------------------------------------

//--------------------------SPHERE----------------------------
void CSphere::Init() {
    //Build(1, 64, 32, 360, 180, false);
    Build(radius, slices, stacks, slicesAng, stacksAng, invert);
}

void CSphere::Params(float Radius, uint Slices, uint Stacks, float SlicesAng, float StacksAng, bool Invert) {
    this->radius    = Radius;
    this->slices    = Slices;
    this->stacks    = Stacks;
    this->slicesAng = SlicesAng;
    this->stacksAng = StacksAng;
    this->invert    = Invert;
}

void CSphere::Build(float Radius,uint Slices,uint Stacks,float SlicesAng,float StacksAng, bool Invert) {
    uint w = Slices + 1;  // mesh width
    uint h = Stacks + 1;  // mesh height
    float x_step = SlicesAng / Slices;
    float y_step = StacksAng / Stacks;

    // Generate vertex + normal + tc array
    std::vector<Vertex> vertices(w * h);

    forXY(w, h) {
        //vec3 v(0,-1, 0);
        vec3 v(0,-Radius, 0);
        v.RotateX( y_step * y);
        v.RotateY( x_step * x);
        vec3 n = normalize(Invert?-v:v);
        vec2 tc = vec2(x/(w-1.f), 1.f - y/(h-1.f));
        Vertex& vertex = vertices[x + y * w];
        vertex.pos = v;                           // vertex
        vertex.nrm = n;                           // normal
        vertex.tc  = tc;                          // uv
    }

    // Generate index array
    std::vector<uint> indices;
    auto AddTriangle = [&](uint v0, uint v1, uint v2) {
        // skip degenerate triangles
        vec3& p0 = vertices[v0].pos;
        vec3& p1 = vertices[v1].pos;
        vec3& p2 = vertices[v2].pos;
        if((p0==p1) || (p1==p2) || (p0==p2)) return;

        indices.push_back(v0);
        indices.push_back(v1);
        indices.push_back(v2);
    };
    auto AddQuad = [&](uint v0, uint v1, uint v2, uint v3) {
        if(!Invert) {
            AddTriangle(v0, v2, v1);
            AddTriangle(v1, v2, v3);
        }else{
            AddTriangle(v0, v1, v2);
            AddTriangle(v1, v3, v2);
        }
    };

    assert(w>1);
    forXY(w-1, h-1) {
        uint pt0 = (x+0)%w + ((y+0)*w);
        uint pt1 = (x+0)%w + ((y+1)*w);
        uint pt2 = (x+1)%w + ((y+0)*w);
        uint pt3 = (x+1)%w + ((y+1)*w);
        AddQuad(pt0, pt1, pt2, pt3);
    }

    //vbo.Data(vertices.data(), (uint32_t)vertices.size(), sizeof(Vertex));
    vbo.Data(vertices);
    ibo.Data(indices.data(),  (uint32_t)indices.size());
    LOGI("Sphere triangle count: %d\n", (int)indices.size()/3);
}
//------------------------------------------------------------

//---------------------------SHAPE----------------------------
//  CShape generates an lathed shape, from an array of vec2 pairs.
//  Each vec2 defines a circle's radius(x) and y-position(y);
//  The Slices parameter defaults to 64.

void CShape::Geometry(Shape shape, uint slices, bool flatX, bool flatY) {
    this->shape  = shape;
    this->slices = slices;
    this->flatX  = flatX;
    this->flatY  = flatY;
}

void CShape::Init() {
//    Bind();
    std::vector<vec2> pairs;
    switch (shape) {
        case CONE      : pairs = {{0,0}, {1,0}, {0,1}};                           break;
        case PIPE      : pairs = {{1,0}, {1,4}};                                  break;
        case CYLINDER  : pairs = {{0,0}, {1,0}, {1,4}, {0,4}};                    break;
        case ARROW     : pairs = {{0,0}, {0.1f,0}, {.1f,2}, {.2f,2}, {0,2.5f}};   break;
        case RING      : pairs = {{1,0}, {1.1f,0}, {1.1f,0.1f}, {1,0.1f}, {1,0}}; break;
        case FAN       : pairs = {{0.1,0}, {1,0}};                                break;
    }
    Build(pairs, slices, flatX, flatY);
}

void CShape::Build(std::vector<vec2>& pairs, uint Slices, bool flatX, bool flatY) {
    bool flip_normals = false;
    float x_step = degrees / Slices;
    uint w = Slices + 1;          // mesh width
    uint h = (uint)pairs.size();  // stacks
    if(flatX) w*=2;
    if(flatY) h*=2;

    std::vector<Vertex> vertices(w * h);
    forXY(w, h) {
        int x2 = flatX ? x/2 : x;
        int y2 = flatY ? y/2 : y;
        vec2 pair = pairs[y2];
        vec3 v(pair, 0);
        v.RotateY(-x_step * x2);
        Vertex& vertex = vertices[x + y * w];
        vertex.pos = v;                           // vertex
        vertex.tc  = vec2(x2/(w-1.f), y2/(h-1.f));  // uv
    }


    // Generate index array
    std::vector<uint> indices;
    auto AddTriangle = [&](uint v0, uint v1, uint v2) {
        //skip degenerate triangles
        vec3& p0 = vertices[v0].pos;
        vec3& p1 = vertices[v1].pos;
        vec3& p2 = vertices[v2].pos;
        if((p0==p1) || (p1==p2) || (p0==p2)) return;

        indices.push_back(v0);
        indices.push_back(v1);
        indices.push_back(v2);
    };
    auto AddQuad = [&](uint v0, uint v1, uint v2, uint v3) {
        AddTriangle(v0, v1, v2);
        AddTriangle(v1, v3, v2);
    };

    forXY(w-1, h-1){
        uint pt0 = (x+0)%w + ((y+0)*w);
        uint pt1 = (x+0)%w + ((y+1)*w);
        uint pt2 = (x+1)%w + ((y+0)*w);
        uint pt3 = (x+1)%w + ((y+1)*w);
        AddQuad(pt0, pt1, pt2, pt3);
    }

    // Generate Normals
    uint face_count = uint(indices.size()/3);
    repeat(face_count) {
        Vertex* vt0 = &vertices[indices[i*3+0]];
        Vertex* vt1 = &vertices[indices[i*3+1]];
        Vertex* vt2 = &vertices[indices[i*3+2]];
        vec3 v01 = vt1->pos - vt0->pos;
        vec3 v02 = vt2->pos - vt0->pos;
        vec3 cross = v01.cross(v02);
        float len = cross.length();
        int odd = i&1; // odd+even triangle per quad
        if(len>0) {  // prevent nan
            vec3 face_normal = cross / len;
            vt0->nrm += face_normal / (1.f+odd);
            vt1->nrm += face_normal /  2.f;
            vt2->nrm += face_normal / (2.f-odd);
        }
    }
    for(auto& v:vertices) v.nrm.normalize();             // normalize all normals
    if(flip_normals)for(auto& v:vertices) v.nrm=-v.nrm;  // flip normals

    vbo.Data(vertices);
    ibo.Data(indices);
    LOGI("Shape triangle count: %d\n", (int)indices.size()/3);
}
//------------------------------------------------------------
