#include "CObject.h"
#include "CCamera.h"
#include "CSkybox.h"
//#include "CSphere.h"
#include "glTF.h"

struct Scene {
    //---- Scene objects ----
    CObject root   = "root";
    CCamera camera = "camera";
    glTF    model  = "glTF";
    //CSphere sphere = "sphere";
    //CCube   cube   = "cube";
    CSkybox skybox = "skybox";
    //-----------------------
    //--- Shared textures ---
    CvkImage vk_skybox_texture;
    //-----------------------
    void Init();
};

void Scene::Init() {
    //--- Scene graph structure ---
    root.Add(camera);
    root.Add(skybox);
    //root.Add(sphere);
    //root.Add(cube);
    root.Add(model);
    //-----------------------------

    // sky
    CCubemap skybox_texture;

//#define SKYBOX
#ifdef  SKYBOX
    skybox_texture.face[FRONT ].Load(SKY"front.jpg",true);
    skybox_texture.face[BACK  ].Load(SKY"back.jpg" ,true);
    skybox_texture.face[BOTTOM].Load(SKY"down.jpg" ,true);
    skybox_texture.face[TOP   ].Load(SKY"up.jpg"   ,true);
    skybox_texture.face[RIGHT ].Load(SKY"right.jpg",true);
    skybox_texture.face[LEFT  ].Load(SKY"left.jpg" ,true);
    //CvkImage vk_skybox_texture(skybox_texture, VK_FORMAT_R8G8B8A8_UNORM, true);          // upload to gpu and generate mipmaps (linear)
    //CvkImage vk_skybox_texture(skybox_texture, VK_FORMAT_R8G8B8A8_SRGB, true);           // upload to gpu and generate mipmaps (gamma)
    CvkImage vk_skybox_texture(skybox_texture, VK_FORMAT_A2B10G10R10_UNORM_PACK32, true);  // upload to gpu and generate mipmaps

#else
    Timer t1;
    skybox_texture.LoadPanorama("Skybox/cloudy.hdr");
    //skybox_texture.LoadPanorama("Skybox/belfast_sunset_puresky_4k.hdr");
    //skybox_texture.LoadPanorama("Skybox/table_mountain_1_puresky_4k.hdr");
    //skybox_texture.LoadPanorama("Skybox/limpopo_golf_course_4k.hdr");
    t1.Print("Panorama");
    Timer t;
    //vk_skybox_texture(skybox_texture, VK_FORMAT_R32G32B32A32_SFLOAT, true);
    //vk_skybox_texture(skybox_texture, VK_FORMAT_R16G16B16A16_SFLOAT, true);
    //vk_skybox_texture(skybox_texture, VK_FORMAT_A2B10G10R10_UNORM_PACK32, true);
    vk_skybox_texture.Data(skybox_texture, VK_FORMAT_E5B9G9R9_UFLOAT_PACK32, true);
    t.Print("RGB9E5");
    //CvkImage vk_skybox_texture(skybox_texture);
#endif

    skybox_texture.Clear();
    model .cubemap = &vk_skybox_texture;
    skybox.cubemap = &vk_skybox_texture;
    //sphere.cubemap = &vk_skybox_texture;

    // model
#define DAMAGED_HELMET
#ifdef  DAMAGED_HELMET
    model.Load("DamagedHelmet/glTF-Binary/DamagedHelmet.glb");
    //model.Load("DamagedHelmet/glTF/DamagedHelmet.gltf");
    model.matrix.RotateY(90);
    model.matrix.RotateX(-90);
    auto* helmet = (CMesh*)model.Find("mesh_helmet_LP_13930damagedHelmet");
    if(helmet) helmet->material.color.emission.set(1,1,1,1);
#else
    model.Load("FlightHelmet/glTF/FlightHelmet.gltf");
    model.matrix.RotateZ(180);
#endif

    camera.orbit_radius = 5;
}
