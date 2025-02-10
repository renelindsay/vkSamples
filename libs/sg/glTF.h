// glTF
// Load a scene from a gltf file
// Load meshes, materials and cameras
// Load BaseColor/Metallic/Roughness Textures

#ifndef GLTF_H
#define GLTF_H

#include "CObject.h"
#include "Mesh.h"
#include "Material.h"
#include "CCamera.h"

#define TINYGLTF_USE_FOPEN            // for android compatibility
#define TINYGLTF_NO_STB_IMAGE_WRITE   // image files are read only
#define TINYGLTF_NOEXCEPTION          // disable exception handling.
#include "tiny_gltf.h"

#undef MOVE_SEMANTICS
//------------------------------------------------------------
#define MOVE_SEMANTICS(CLASS)                                \
    public:                                                  \
        CLASS(const CLASS&) = delete;                        \
        CLASS& operator=(const CLASS&) = delete;             \
        CLASS(CLASS&& other) : CLASS() { swap(other); }      \
        CLASS& operator=(CLASS&& other) {                    \
            if(this != &other) swap(other);                  \
            return *this;                                    \
        }                                                    \
    protected:                                               \
        void swap(CLASS& other);
//------------------------------------------------------------

class glTF : public CObject {
    MOVE_SEMANTICS(glTF)
    tinygltf::Model* p_model = nullptr;
    std::vector<CObject*> nodes;
    std::vector<CCamera*> cameras;
    std::vector<Material> materials;
    std::vector<CvkImage> vkImages;

    void load_materials(const char* path);                        // load all textures
    void load_object(CObject* parent, tinygltf::Node& t_node);    // load nodes recursively
    mat4     get_matrix(const tinygltf::Node& t_node);            // load node's transform matrix
    CMesh*   get_mesh  (const tinygltf::Mesh& t_mesh, uint prim=0);
    CCamera* get_camera(const tinygltf::Camera& t_cam);

  public:
    glTF(const char* name="glTF") : CObject(name) { type = ntGLTF; }
    ~glTF(){ Clear(); }
    void Clear();
    CNode* Load(const char* filename, CObject* parent = nullptr);

    CPipeline* pipeline = 0;
    CvkImage*  cubemap = 0;
    CCamera*   camera = 0;
    bool mipmap = true;
};


#endif
