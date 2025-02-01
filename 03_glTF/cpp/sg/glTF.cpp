#include "glTF.h"

#define TINYGLTF_IMPLEMENTATION
#include "tiny_gltf.h"
//#include "matrix.h"

// move construction
void glTF::swap(glTF& other) {
    std::swap(pipeline, other.pipeline);
    std::swap(cubemap,  other.cubemap);
    std::swap(camera,   other.camera);
    std::swap(mipmap,   other.mipmap);
    std::swap(nodes,    other.nodes);
    std::swap(cameras,  other.cameras);
    std::swap(materials,other.materials);
    std::swap(vkImages, other.vkImages);
    std::swap(p_model,  other.p_model);
}

void glTF::Clear() {
    for(auto& node : nodes) { delete node; }
    nodes.clear();
    materials.clear();
    matrix.SetIdentity();
}

CNode* glTF::Load(const char* filename, CObject* parent) {
    if(!parent) parent = this;  // by default, use the scene node as root

    // get path to files
    std::string fname = filename;
    size_t slash = fname.find_last_of("/\\")+1;
    size_t dot   = fname.find_last_of(".");

    std::string path = fname.substr(0, slash);
    std::string ext  = fname.substr(dot + 1);
    std::string name = fname.substr(slash, dot - slash);

    LOG("\n");
    LOGI("Loading glTF Scene : %s\n", filename);
//    Clear();
    tinygltf::TinyGLTF t_loader;
    tinygltf::Model    t_model;
    std::string err;
    std::string warn;

    bool ret = false;
    if(ext == "gltf") ret = t_loader.LoadASCIIFromFile (&t_model, &err, &warn, filename); else // ASCII
    if(ext == "glb")  ret = t_loader.LoadBinaryFromFile(&t_model, &err, &warn, filename);      // Binary

    if (!warn.empty()) { LOGW("Warn: %s\n", warn.c_str()); }
    if (!err.empty())  { LOGE("Error: %s\n", err.c_str()); }
    if (!ret) { LOGE("Failed to parse glTF\n");  return nullptr; }

    p_model = &t_model;
    load_materials(path.c_str());  // load all textures

    tinygltf::Scene& t_scene = t_model.scenes[t_model.defaultScene];  // assume one scene
    uint32_t node_cnt = (uint32_t)t_scene.nodes.size();               // load all nodes
    repeat(node_cnt) {
        uint t_node_id = (uint)t_scene.nodes[i];
        tinygltf::Node t_node = t_model.nodes[t_node_id];
        load_object(parent, t_node);
    }

    p_model = nullptr;
    return this;
}

void glTF::load_materials(const char* path) {
    size_t count = p_model->textures.size();
    std::vector<CImage> images(count);
    vkImages.resize(count);
    uint inx = 0;
    for(const auto& t_tex : p_model->textures) {
        CImage* img = &images[inx++];
        tinygltf::Image t_img = p_model->images[t_tex.source];
        std::string fname = t_img.uri;

        if(fname.empty()) {  // load texture from buffer in .glb
            tinygltf::BufferView& bv = p_model->bufferViews[t_img.bufferView];
            uint8_t* buf = p_model->buffers[bv.buffer].data.data() + bv.byteOffset;
            int      len = (int)bv.byteLength;
            img->Load_from_mem(buf, len, false);
        } else {  // load texture from external file
            fname = path + fname;
            img->Load(fname.c_str(), false);
        }

        if(t_tex.sampler >= 0) {
            tinygltf::Sampler t_smp = p_model->samplers[t_tex.sampler];
            //if(t_smp.magFilter == TINYGLTF_TEXTURE_FILTER_NEAREST) img->magFilter = NEAREST;
            if(t_smp.magFilter == TINYGLTF_TEXTURE_FILTER_LINEAR)  img->magFilter  = LINEAR;
            if(t_smp.wrapS == TINYGLTF_TEXTURE_WRAP_REPEAT)        img->wrapMode_U = wmREPEAT;
            if(t_smp.wrapT == TINYGLTF_TEXTURE_WRAP_REPEAT)        img->wrapMode_V = wmREPEAT;

            // TODO
            //t_smp.minFilter;
            assert(t_smp.wrapS != TINYGLTF_TEXTURE_WRAP_MIRRORED_REPEAT && "Mirrored repeat not yet implemented");
            assert(t_smp.wrapT != TINYGLTF_TEXTURE_WRAP_MIRRORED_REPEAT && "Mirrored repeat not yet implemented");
        }
    }

    // load materials
    for(const auto& t_mat : p_model->materials) {
        LOGI("  Material: %s\n", t_mat.name.c_str());
        //for(const auto& val : t_mat.values) { LOGI("     value: %s\n", val.first.c_str()); }

        materials.emplace_back();
        Material& mat=materials.back();

        //Material mat;
        mat.name = t_mat.name;
        auto val_end = t_mat.values.end();

        // --- factors ---
        auto facColor = t_mat.values.find("baseColorFactor");
        auto facMetal = t_mat.values.find("metallicFactor");
        auto facRough = t_mat.values.find("roughnessFactor");

        if (facColor != val_end) {
            tinygltf::ColorValue c = facColor->second.ColorFactor();
            mat.color.albedo.set((float)c[0], (float)c[1], (float)c[2], (float)c[3]);
        }
        if (facMetal != val_end) mat.color.orm.z = (float)facMetal->second.Factor();
        if (facRough != val_end) mat.color.orm.y = (float)facRough->second.Factor();
        // ----------------

        // --- textures ---
        //auto val_end = t_mat.values.end();
        auto add_end = t_mat.additionalValues.end();
        auto texCol  = t_mat.values.find("baseColorTexture");
        auto texMR   = t_mat.values.find("metallicRoughnessTexture");
        auto texNrm  = t_mat.additionalValues.find("normalTexture");
        auto texEm   = t_mat.additionalValues.find("emissiveTexture");
        auto texAO   = t_mat.additionalValues.find("occlusionTexture");

        // Get texture index (or -1 if none)
        int iCol=(texCol != val_end) ? texCol->second.TextureIndex() : -1;
        int iNrm=(texNrm != add_end) ? texNrm->second.TextureIndex() : -1;
        int iEmi=(texEm  != add_end) ? texEm ->second.TextureIndex() : -1;
        int iAO =(texAO  != add_end) ? texAO ->second.TextureIndex() : -1;
        int iMR =(texMR  != val_end) ? texMR ->second.TextureIndex() : -1;

        // if AO is in a separate texture, merge it with the MR texture.
        if((iAO != iMR) && (iAO>-1) && (iMR>-1)) {
            CImage* imgAO = &images[iAO];
            CImage* imgMR = &images[iMR];
            imgAO->sRGBtoUNORM();
            imgMR->Blend(*imgAO,1,0,0);
            imgMR->colorspace = csUNORM;
        }
        int iOrm = iMR;

        // Gamma correction
        if(iCol>=0) images[iCol].colorspace = csSRGB;    // albedo   (apply gamma correction)
        if(iOrm>=0) images[iOrm].colorspace = csUNORM;   // ORM      (DONT gamma correct)
        if(iNrm>=0) images[iNrm].colorspace = csUNORM;   // normals  (DONT gamma correct)
        if(iEmi>=0) images[iEmi].colorspace = csSRGB;    // emission (apply gamma correction)

        //if(iCol>=0) images[iCol].sRGBtoUNORM();
        //if(iEmi>=0) images[iEmi].sRGBtoUNORM();

        // Upload to GPU and geneate mipmaps
        if(iCol>=0) vkImages[iCol].Data(images[iCol], mipmap);
        if(iOrm>=0) vkImages[iOrm].Data(images[iOrm], mipmap);
        if(iNrm>=0) vkImages[iNrm].Data(images[iNrm], mipmap);
        if(iEmi>=0) vkImages[iEmi].Data(images[iEmi], mipmap);

        if(iCol>=0) mat.texture.albedo   = &(vkImages[iCol]);
        if(iOrm>=0) mat.texture.orm      = &(vkImages[iOrm]);
        if(iNrm>=0) mat.texture.normal   = &(vkImages[iNrm]);
        if(iEmi>=0) mat.texture.emission = &(vkImages[iEmi]);
    }
}

void glTF::load_object(CObject* parent, tinygltf::Node& t_node) {
    mat4 m = get_matrix(t_node);

    if(t_node.camera >= 0) { // node has a camera
        tinygltf::Camera t_cam = p_model->cameras[t_node.camera];
        CCamera* camera = get_camera(t_cam);
        if(camera->name.size()==0) camera->name = "camera";
        camera->name = t_cam.name;
        nodes.push_back(camera);
        cameras.push_back(camera);
        camera->matrix = m;
        parent->Add(*camera);
    }

    if(t_node.mesh >= 0) {  // node has a mesh
        tinygltf::Mesh t_mesh = p_model->meshes[t_node.mesh];
        uint prim_cnt = (uint)t_mesh.primitives.size();
        repeat(prim_cnt) {  // multiple primitives per model

            CMesh* mesh = get_mesh(t_mesh, i);
            mesh->name = t_mesh.name;
            if(mesh->name.size()==0) mesh->name = "mesh";
            if(prim_cnt > 1) mesh->name += "_"+std::to_string(i);
            nodes.push_back(mesh);
            mesh->matrix = m;
            parent->Add(*mesh);

            mesh->pipeline = pipeline;  // link model to default pipeline
            mesh->cubemap  = cubemap;   // link model to default cubemap
        }
    }


    if((t_node.mesh<0) && (t_node.camera<0)) { // node is empty
        CObject* node = new CObject;
        node->name = t_node.name;
        if(node->name.size()==0) node->name = "node";
        nodes.push_back(node);
        node->matrix = m;
        parent->Add(*node);
    }

    CObject* node = nodes.back();  // use the last node added as next parent
    for(const auto& t_child_id : t_node.children) {
        tinygltf::Node& t_child = p_model->nodes[t_child_id];
        load_object(node, t_child);
    }
}

mat4 glTF::get_matrix(const tinygltf::Node& t_node) {
    mat4 m;
    if(t_node.matrix.size()==16) {
        repeat(16) m[i] = (float)t_node.matrix[i];
    } else {
        mat4 s, r, t;
        if(t_node.scale.size() == 3) {
            s.Scale((float)t_node.scale[0], (float)t_node.scale[1], (float)t_node.scale[2]);
        }
        if(t_node.rotation.size() == 4) {
           quat q((float)t_node.rotation[0], (float)t_node.rotation[1], (float)t_node.rotation[2], (float)t_node.rotation[3]);
           r.from_quat(q);
           r.RotateZ(180);
        }
        if(t_node.translation.size() == 3) {
            t.Translate((float)t_node.translation[0], (float)t_node.translation[1], (float)t_node.translation[2]);
        }
        m = s * r * t;
    }
    //m.RotateY(180);
    return m;
}

CCamera* glTF::get_camera(const tinygltf::Camera& t_cam) {
    CCamera* camera = new CCamera;
    LOGI("  Camera: %s\n", t_cam.name.c_str());

    if(t_cam.type.compare("perspective")==0) {
        const tinygltf::PerspectiveCamera& pc = t_cam.perspective;
        camera->SetPerspective((float)pc.aspectRatio, (float)pc.yfov, (float)pc.znear, (float)pc.zfar);
    } else {
        const tinygltf::OrthographicCamera& oc = t_cam.orthographic;
        camera->SetOrthographic((float)-oc.xmag, (float)oc.xmag, (float)-oc.ymag, (float)oc.ymag, (float)oc.znear, (float)oc.zfar);
    }
    return camera;
}

/*
float asFloat(char* addr, int ctype){
    if(ctype == TINYGLTF_COMPONENT_TYPE_FLOAT) return *((float*)addr);
    if(ctype == TINYGLTF_COMPONENT_TYPE_SHORT) return *((short*)addr);
    if(ctype == TINYGLTF_COMPONENT_TYPE_BYTE ) return *((uint8_t*)addr);
    return 0;
}

vec4 atInx(int type, int ctype, void* buf, uint32_t inx) {  // byte/short/float
    uint ct_size = tinygltf::GetComponentSizeInBytes(ctype);
    uint stride = type * ct_size;
    char* addr = (char*)buf + inx*stride;
    vec4 v;        v.x = asFloat(addr+ct_size*0, ctype);
    if (ctype ==2) v.y = asFloat(addr+ct_size*1, ctype);
    if (ctype >=3) v.z = asFloat(addr+ct_size*2, ctype);
    if (ctype >=4) v.w = asFloat(addr+ct_size*3, ctype);
    return v;
}
*/

CMesh* glTF::get_mesh(const tinygltf::Mesh& t_mesh, uint prim) {
    CMesh* mesh = new CMesh;
    LOGI("  Mesh: %s\n", t_mesh.name.c_str());

    //WIP
    {
        const tinygltf::Primitive& t_primitive = t_mesh.primitives[prim];

        auto at_end = t_primitive.attributes.end();
        auto vt_pos = t_primitive.attributes.find("POSITION"  );
        auto vt_nrm = t_primitive.attributes.find("NORMAL"    );
        auto vt_tan = t_primitive.attributes.find("TANGENT"   );
        auto vt_tc0 = t_primitive.attributes.find("TEXCOORD_0");
        auto vt_tc1 = t_primitive.attributes.find("TEXCOORD_1");  // TODO
        auto vt_col = t_primitive.attributes.find("COLOR_0"   );

        vec3*  pos = nullptr;  // position
        size_t pos_stride = sizeof(vec3);
        if (vt_pos != at_end) {
            tinygltf::Accessor& a_pos = p_model->accessors[vt_pos->second];
            tinygltf::BufferView& bv_pos = p_model->bufferViews[a_pos.bufferView];
            uint8_t* buf = p_model->buffers[bv_pos.buffer].data.data();
            pos = (vec3*)(buf + a_pos.byteOffset + bv_pos.byteOffset);
            if(bv_pos.byteStride > 0) pos_stride = bv_pos.byteStride;
        }

        vec3*  nrm = nullptr;  // normal
        size_t nrm_stride = sizeof(vec3);
        if (vt_nrm != at_end) {
            tinygltf::Accessor& a_nrm = p_model->accessors[vt_nrm->second];
            tinygltf::BufferView& bv_nrm = p_model->bufferViews[a_nrm.bufferView];
            uint8_t* buf = p_model->buffers[bv_nrm.buffer].data.data();
            nrm = (vec3*)(buf + a_nrm.byteOffset + bv_nrm.byteOffset);
            if(bv_nrm.byteStride > 0) nrm_stride = bv_nrm.byteStride;
        }
/*
        vec3*  tan = nullptr;  // tangent
        size_t tan_stride = sizeof(vec3);
        if (vt_tan != at_end) {
            tinygltf::Accessor& a_tan = p_model->accessors[vt_tan->second];
            tinygltf::BufferView& bv_tan = p_model->bufferViews[a_tan.bufferView];
            uint8_t* buf = p_model->buffers[bv_tan.buffer].data.data();
            tan = (vec3*)(buf + a_tan.byteOffset + bv_tan.byteOffset);
            if(bv_tan.byteStride > 0) tan_stride = bv_tan.byteStride;
        }
*/
        vec2*  tc0 = nullptr;  // tex-coord
        size_t tc0_stride = sizeof(vec2);
        if (vt_tc0 != at_end) {
            tinygltf::Accessor& a_tc0 = p_model->accessors[vt_tc0->second];
            assert(a_tc0.componentType == TINYGLTF_PARAMETER_TYPE_FLOAT);
            tinygltf::BufferView& bv_tc0 = p_model->bufferViews[a_tc0.bufferView];
            uint8_t* buf = p_model->buffers[bv_tc0.buffer].data.data();
            tc0 = (vec2*)(buf + a_tc0.byteOffset + bv_tc0.byteOffset);
            if(bv_tc0.byteStride > 0) tc0_stride = bv_tc0.byteStride;
        }
/*
        vec4*  col = nullptr;  // color
        size_t col_stride = sizeof(vec4);
        if (vt_col != at_end) {
            tinygltf::Accessor& a_col = p_model->accessors[vt_col->second];

            //assert(a_col.type = 4);
            //assert(a_col.componentType == TINYGLTF_PARAMETER_TYPE_FLOAT);
            //uint ctype = a_col.componentType;
            //uint ct_size = tinygltf::GetComponentSizeInBytes(ctype);

            tinygltf::BufferView& bv_col = p_model->bufferViews[a_col.bufferView];
            uint8_t* buf = p_model->buffers[bv_col.buffer].data.data();
            col = (vec4*)(buf + a_col.byteOffset + bv_col.byteOffset);
            if(bv_col.byteStride > 0) tc0_stride = bv_col.byteStride;
        }
*/

        // Vertex Buffer object
        uint vrt_count = (uint)p_model->accessors[vt_pos->second].count;
        std::vector<Vertex> vertices(vrt_count);
        repeat(vrt_count) {
            Vertex& vert = vertices[i];
            if(pos) vert.pos = *(vec3*)((char*)pos + (i*pos_stride));
            if(nrm) vert.nrm = *(vec3*)((char*)nrm + (i*nrm_stride));
            if(tc0) vert.tc  = *(vec2*)((char*)tc0 + (i*tc0_stride));
            //if(tan) vert.tan  = *(vec3*)((char*)tan + (i*tan_stride));  // use tangent if available
            //else    vert.tan  = cross(up, vertices[i].norm);            // else align to model
            //vert.col = vec4(1,1,1,1);
        }

        //mesh->vbo.Data(vertices.data(), vrt_count, sizeof(Vertex));
        mesh->vbo.Data(vertices);

        // Index array
        if(t_primitive.indices >= 0) {
            tinygltf::Accessor& a_inx = p_model->accessors[t_primitive.indices];
            uint ctype = a_inx.componentType;               // type: byte/short/int
            uint ct_size = tinygltf::GetComponentSizeInBytes(ctype);  // size:   1 /  2  / 4
            tinygltf::BufferView& bv_inx = p_model->bufferViews[a_inx.bufferView];
            uint8_t* buf = p_model->buffers[bv_inx.buffer].data.data();

            std::vector<uint> indices(a_inx.count);
            void* inx = (void*)(buf + a_inx.byteOffset + bv_inx.byteOffset);
            if(ct_size == 1) repeat(a_inx.count) indices[i] = ((uint8_t* )inx)[i];  // index of bytes
            if(ct_size == 2) repeat(a_inx.count) indices[i] = ((uint16_t*)inx)[i];  // index of shorts
            if(ct_size == 4) repeat(a_inx.count) indices[i] = ((uint32_t*)inx)[i];  // index of ints
            mesh->ibo.Data(indices.data(), (uint32_t)indices.size());
        }

        if(t_primitive.material >= 0) {
            mesh->material = materials[t_primitive.material];
        }

    }

    return mesh;
}
