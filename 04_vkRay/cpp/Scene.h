#include "CObject.h"
#include "CCamera.h"
#include "CSkybox.h"
#include "glTF.h"
#include "Light.h"

struct Scene {
    //---- Scene objects ----
    CObject root   = "root";
    CObject camX   = "camX";
    CObject camY   = "camY";
    CCamera camera = "camera";
    glTF    model  = "glTF";
    CSphere sphere = "sphere";
    //CCube   cube   = "cube";
    CSkybox skybox = "skybox";
    CLight  light  = "light";
    //-----------------------
    //--- Shared textures ---
    CvkImage vk_skybox_texture;
    //-----------------------
    void Init();
};

void Scene::Init() {
    //--- Scene graph structure ---
    root.Add(camX);
    camX.Add(camY);
    camY.Add(camera);
    root.Add(skybox);
    //root.Add(sphere);
    //root.Add(cube);
    root.Add(model);
    root.Add(light);
    //-----------------------------

    //--- SKYBOX ---
    CCubemap skybox_texture;
    skybox_texture.LoadPanorama("Skybox/rooitou_park_4k.hdr");
    vk_skybox_texture.Data(skybox_texture, VK_FORMAT_E5B9G9R9_UFLOAT_PACK32, true);

//#define SKYBOX
#ifdef  SKYBOX
//#define SKY "Skybox/blueskies/"
//#define SKY "Skybox/miramar/"
#define SKY "Skybox/stormydays/"
    skybox_texture.face[FRONT ].Load(SKY"front.jpg",true);
    skybox_texture.face[BACK  ].Load(SKY"back.jpg" ,true);
    skybox_texture.face[BOTTOM].Load(SKY"down.jpg" ,true);
    skybox_texture.face[TOP   ].Load(SKY"up.jpg"   ,true);
    skybox_texture.face[RIGHT ].Load(SKY"right.jpg",true);
    skybox_texture.face[LEFT  ].Load(SKY"left.jpg" ,true);
    vk_skybox_texture.Data(skybox_texture, VK_FORMAT_A2B10G10R10_UNORM_PACK32, true);
#endif

//#define ONOFF
#ifdef ONOFF
#define SKY "Skybox/sunvec/"
    skybox_texture.face[FRONT ].Load(SKY"off.png" ,true);
    skybox_texture.face[BACK  ].Load(SKY"off.png" ,true);
    skybox_texture.face[BOTTOM].Load(SKY"off.png" ,true);
    skybox_texture.face[TOP   ].Load(SKY"off.png" ,true);
    skybox_texture.face[RIGHT ].Load(SKY"off.png" ,true);
    skybox_texture.face[LEFT  ].Load(SKY"on2.png" ,true);
    vk_skybox_texture.Data(skybox_texture, VK_FORMAT_A2B10G10R10_UNORM_PACK32, true);
#endif

    RGBA32f flux;
    light.position = skybox_texture.GetSunVec(&flux);
    light.color = flux / 1024.f;

#ifdef ONOFF
    light.color = flux * 64.f;
#endif

    skybox_texture.Clear();                                                        // delete cpu-side buffers
    //--------------
    model .cubemap = &vk_skybox_texture;
    skybox.cubemap = &vk_skybox_texture;
    sphere.cubemap = &vk_skybox_texture;

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

    sphere.material.color.orm.set(1,1,0);
    camera.matrix.Translate(0, 0, 4);   // camera offset
};
