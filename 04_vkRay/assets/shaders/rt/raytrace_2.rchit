#version 460
#extension GL_EXT_ray_tracing : require
#extension GL_GOOGLE_include_directive : enable

#include "unpack.glsl"  // unpack vertex struct
//#include "utils.glsl"

struct Camera {
    mat4  view;
    mat4  proj;
    mat4  viewInverse;
    mat4  projInverse;
    uint  flags;
    int   sky;
    uint  random;
};

struct Material { 
    vec4 albedo;
    vec4 emission;
    vec4 normal;  // not used
    vec4 orm;
    int  tex_albedo;
    int  tex_emission;
    int  tex_normal;
    int  tex_orm;
};

struct Model {
    mat4 matrix;
    Material material;
};

struct Light {
    bool enabled;
    vec4 color;
    vec4 position;  // or direction, if w=0
};

struct RayPayload {
    //int hit;

    float distance;
    //vec3 hitpos;
    //vec3 normal;
    //vec3 diffuse;
    //vec4 albedo;
    //vec3 illum;
    //vec3 orm;
    
    vec3 color;
};

layout(location = 0) rayPayloadInEXT RayPayload payload;  // to rgen
layout(location = 1) rayPayloadEXT   bool       isShadow; // from shadow rmiss

// input from Vulkan
layout(binding = 0, set = 0) uniform accelerationStructureEXT tlas;
layout(binding = 2, set = 0) uniform Cameras  { Camera camera; };            // camera ubo
layout(binding = 3, set = 0) buffer  Models   { Model  model;  } models [];  // mesh ubo
layout(binding = 6, set = 0) uniform sampler2D[]                 samplers;
//layout(binding = 7, set = 0) uniform Lights   { Light light;   };

hitAttributeEXT vec2 attribs;

void main() { 
    //payload.hit = 1; 
    
    Vertex v[3] = getTriangle();
    const vec3 bary = vec3(1.0 - attribs.x - attribs.y, attribs.x, attribs.y);
    vec3 normal = normalize(v[0].nrm * bary.x + v[1].nrm * bary.y + v[2].nrm * bary.z);
    vec2 tc     =           v[0].tc  * bary.x + v[1].tc  * bary.y + v[2].tc  * bary.z;
    vec3 hitpos =           v[0].pos * bary.x + v[1].pos * bary.y + v[2].pos * bary.z;
      
    Model model = models[gl_InstanceID].model;
    mat3 m3 = mat3(model.matrix);
    
    
    //vec3 world_hitpos = gl_WorldRayOriginEXT + gl_WorldRayDirectionEXT * gl_HitTEXT;  // less accurate
    vec3 world_hitpos = (model.matrix * vec4(hitpos, 1.0)).xyz;                      // more accurate
    vec3 world_normal = normalize(m3 * normal);
    
    //vec3 ray_dir = normalize(gl_WorldRayDirectionEXT);
    //vec3 reflect_vec = reflect(ray_dir, world_normal);
    
    //---- Textures ----
    Material material = model.material;
    vec4 albedo   = material.albedo;
    vec4 emission = material.emission;
    vec4 normap   = vec4(0.5,0.5,1.0,0.0);
    vec4 orm      = material.orm;
         
    int alb_inx = material.tex_albedo;
    int emi_inx = material.tex_emission;
    int nor_inx = material.tex_normal;
    int orm_inx = material.tex_orm;
    
    if(alb_inx>=0) albedo   *= texture(samplers[alb_inx], tc);
    if(emi_inx>=0) emission *= texture(samplers[emi_inx], tc);
    if(nor_inx>=0) normap    = texture(samplers[nor_inx], tc);
    if(orm_inx>=0) orm      *= texture(samplers[orm_inx], tc);   
    //-------------------
    
    //---- Reflections ----
    
    
    //---------------------
   
    //---- Light ----
    Light light;
    light.enabled = true;
    light.color = vec4(1,1,1,1);
    light.position = vec4(1,0,0,0);
    vec3 light_vec = normalize(light.position.xyz);
    vec3 illum = vec3(0.01);
    //---------------
/*    
    //---- Shadows ----
    float tmin = 0.001;
    float tmax = 1000.0;
    vec3 bias = world_normal * tmin;
    vec3 origin = world_hitpos + bias;
    isShadow = true;
    
    uint rayFlags = gl_RayFlagsTerminateOnFirstHitEXT | gl_RayFlagsOpaqueEXT | gl_RayFlagsSkipClosestHitShaderEXT;
    uint cullMask = 0xFF;
    uint sbtOffset = 1;
    uint sbtStride = 0;
    uint missIndex = 2;
    traceRayEXT(tlas, rayFlags, cullMask, sbtOffset, sbtStride, missIndex, origin, tmin, light_vec, tmax, 1);  // shadow ray
    
    float dotp = dot(light_vec, world_normal);
    if (!isShadow) illum = dotp * light.color.xyz;
    //-----------------
*/    
    
    payload.distance = gl_HitTEXT;    
    //payload.hitpos   = world_hitpos;
    
    //payload.color = world_hitpos;
    //payload.color = normap.rgb;
    payload.color = albedo.rgb * illum + emission.rgb;

    //payload.color = vec3(1,0,0);
    
}
