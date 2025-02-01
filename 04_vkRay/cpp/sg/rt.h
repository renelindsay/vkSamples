#ifndef RT_H
#define RT_H

#include "CDevices.h"
//#include "CRenderpass.h"
#include "Swapchain.h"
#include "CObject.h"
#include "CCamera.h"
//#include "CSkybox.h"
//#include "glTF.h"
#include "Light.h"

class RT {

public:
    VKRay vkray;
    std::vector<CObject*> meshList;
/*
    //--------------------
    rtImageList imageList;
    void AddMaterial(Material& mat, uboData& ubo){
        imageList.AddMaterial(mat, ubo);
    }
    //--------------------
*/

    void Init(CQueue& queue, CCamera& camera, VkImageView target) {
        vkray.Init(queue);
        CObject& root = camera.GetRoot();
        root.FindAll(ntSKYBOX)[0]->AddToBLAS(vkray);
        meshList = root.GetRenderList();
        for(auto item : meshList){ item->AddToBLAS(vkray); }
        vkray.BuildAS();    
        vkray.m_target = target;
        vkray.m_camera = &camera.cam_ubo;
        CLight* light = (CLight*)root.FindAll(ntLIGHT)[0];
        vkray.m_light = &light->light_ubo;       
        vkray.CreateDescriptorSet();
        vkray.CreatePipeline();
    }

    void Update() {
        for(auto item : meshList){ item->UpdateBLAS(vkray); }
        vkray.UpdateTLAS();
    }

    void Render(CCamera& camera, Swapchain& swapchain) {
        CObject& root = camera.GetRoot();
        root.Transform_nodes();
        swapchain.AcquireNext();

        CvkImage& attachment = swapchain.att_images[0];
        attachment.SetLayout(VK_IMAGE_LAYOUT_GENERAL);
        vkray.SetRenderTarget(attachment.view);

        VkExtent2D ext  = swapchain.GetExtent();
        float w = (float)ext.width;
        float h = (float)ext.height;
        float aspect = w / h;
        camera.SetPerspective(aspect, 40.f, 0.1f, 1000);
        //camera->flags = window.flags;

        auto cmd  = swapchain.BeginCmd();
        auto swap = swapchain.CurrBuffer();
        camera.Apply(swap.fence);

        vkray.BindDS(cmd);
        vkray.TraceRays(cmd, ext);
        attachment.Blit(cmd, swap.image, ext, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);
        swapchain.EndCmd();
        swapchain.Submit();
        swapchain.Wait();
    }
    
};
    
#endif
