#version 460
#extension GL_EXT_ray_tracing : require
#extension GL_GOOGLE_include_directive : enable

#include "unpack.glsl"  // unpack vertex struct
#include "utils.glsl"

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
layout(location = 2) rayPayloadEXT   RayPayload payload2; // from reflect ray
layout(location = 3) rayPayloadEXT   RayPayload payload3; // from diffuse ray

// input from Vulkan
layout(binding = 0, set = 0) uniform accelerationStructureEXT tlas;
layout(binding = 2, set = 0) uniform Cameras  { Camera camera; };            // camera ubo
layout(binding = 3, set = 0) buffer  Models   { Model  model;  } models [];  // mesh ubo
layout(binding = 6, set = 0) uniform sampler2D[]                 samplers;
layout(binding = 7, set = 0) uniform Lights   { Light light;   };

hitAttributeEXT vec2 attribs;

//---------------------------------------------------------------------------------------

// Distribute rays evenly in the hemisphere where the z-axis is the surface normal vector.
// pfrac is the pitch fracion, with range 0 - 1 for 0 to 90 degrees.
// With evenly spaced pfrac, the rays should contribute equally to the diffuse light.

vec3 Diffuse(float pfrac) { 
    float pitch = asin(sqrt(pfrac));     
    float head  = rand() * pi2;
    vec3 v = vec3(0,0,1);      
    v = RotateY(v, pitch);
    v = RotateZ(v, head);
    return v;
}

//---------------------------------------------------------------------------------------

void main() { 
    seed += camera.random;    
    //vec3 dir = gl_WorldRayDirectionEXT;   
    //seed += int(dir.x + dir.y + dir.z);  // randomize
    
    //payload.hit = 1; 
    
    Vertex v[3] = getTriangle();
    const vec3 bary = vec3(1.0 - attribs.x - attribs.y, attribs.x, attribs.y);
    vec3 normal = normalize(v[0].nrm * bary.x + v[1].nrm * bary.y + v[2].nrm * bary.z);
    vec2 tc     =           v[0].tc  * bary.x + v[1].tc  * bary.y + v[2].tc  * bary.z;
    vec3 hitpos =           v[0].pos * bary.x + v[1].pos * bary.y + v[2].pos * bary.z;
      
    Model model = models[gl_InstanceID].model;
    mat3 m3 = mat3(model.matrix);
    
    
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
    //if(nor_inx>=0) normap    = texture(samplers[nor_inx], tc);
    if(orm_inx>=0) orm      *= texture(samplers[orm_inx], tc);   
    //-------------------
    
    float roughness = orm.g * orm.g;
    //float roughness = orm.g;
    float metalness = orm.b;
    
    //vec3 world_hitpos = gl_WorldRayOriginEXT + gl_WorldRayDirectionEXT * gl_HitTEXT;  // less accurate
    vec3 world_hitpos = (model.matrix * vec4(hitpos, 1.0)).xyz;                      // more accurate
    vec3 world_normal = normalize(m3 * normal); 
    
    //---- Normal Map ----     
    if(nor_inx>=0) {
        normap = texture(samplers[nor_inx], tc);
        mat3 tangent_space = look(world_normal, m3[1]);      // TBN matrix
        vec3 normalvec = normalize(normap * 2.0 - 1.0).xyz;  // rgb->xyz
        vec3 norm = normalize(tangent_space * normalvec);    // per-fragment normal, using normal-map
        world_normal = norm;
    }    
    mat3 tangent_space = look(world_normal, m3[1]);   
    //--------------------
    
    //----traceParams ----
    float tmin     = 0.001;
    float tmax     = 1000.0;
    uint cullMask  = 0xFF;
    uint sbtOffset = 1;
    uint sbtStride = 0;
    uint missIndex = 1;
    uint rayFlags  = gl_RayFlagsOpaqueEXT;
    vec3 bias = world_normal * tmin;
    vec3 origin = world_hitpos + bias;   
    //--------------------
    
    //---- Specular ----
    int SPEC_RAYS  = 4;
    missIndex = 0;
    vec3 specular_light = vec3(0);
    vec3 ray_dir = gl_WorldRayDirectionEXT;
    vec3 reflect_vec = reflect(ray_dir, world_normal);
    
    if(roughness < 0.02) {  //for smooth surfaces, use only one specular ray    
        traceRayEXT(tlas, rayFlags, cullMask, sbtOffset, sbtStride, missIndex, origin, tmin, reflect_vec, tmax, 2);
        specular_light = payload2.color;
        //specular_light = vec3(1,0,0);
    } else {        
        for(int i=0; i<SPEC_RAYS; ++i) {
            float pfrac = float(i+1)/(SPEC_RAYS+2);
            vec3 diffuse_vec = tangent_space * Diffuse(pfrac);      
            vec3 specular_vec = mix(reflect_vec, diffuse_vec, roughness);
            traceRayEXT(tlas, rayFlags, cullMask, sbtOffset, sbtStride, missIndex, origin, tmin, specular_vec, tmax, 2);  // reflect ray
            vec3 flux = min(payload2.color, 16);   // reduce fireflies           
            specular_light += flux;
        }    
        specular_light /= SPEC_RAYS;
    }
    //------------------ 
    
    //---- Diffuse ----
    int DIF_RAYS = 8;
    missIndex = 1;
    vec3 diffuse_light = vec3(0);   
    for(int i=0; i<DIF_RAYS; ++i) {
        float pfrac = float(i+1)/(DIF_RAYS+2);      
        vec3 diffuse_vec = tangent_space * Diffuse(pfrac);
        traceRayEXT(tlas, rayFlags, cullMask, sbtOffset, sbtStride, missIndex, origin, tmin, diffuse_vec, tmax, 3);  // diffuse ray
        vec3 flux = min(payload3.color, 8);  // reduce fireflies
        diffuse_light += flux;
    }
    diffuse_light /= DIF_RAYS;
    //-----------------
    
   
    //---- Light ----
    //Light light;
    //light.enabled = true;
    //light.color = vec4(1,1,1,1);
    //light.position = vec4(1,0,0,0);
    vec3 light_vec = normalize(light.position.xyz);
    light_vec += rands3d() * sin(0.57*toRad);  // jitter by sun solid angle
    vec3 illum = vec3(0.02);                   // ambient light
    //---------------
    
    //---- Shadows ----
    float dotp = dot(light_vec, world_normal); 
    if(dotp>0) {
        isShadow = true;
        rayFlags = gl_RayFlagsTerminateOnFirstHitEXT | gl_RayFlagsOpaqueEXT | gl_RayFlagsSkipClosestHitShaderEXT;
        cullMask = 0xFF;
        sbtOffset = 1;
        sbtStride = 0;
        missIndex = 2;
        traceRayEXT(tlas, rayFlags, cullMask, sbtOffset, sbtStride, missIndex, origin, tmin, light_vec, tmax, 1);  // shadow ray
        if(!isShadow) illum += dotp * light.color.xyz;
    }
    //-----------------  
    
    diffuse_light = diffuse_light * DIF_RAYS + illum;
    diffuse_light /= DIF_RAYS+1;
    
       
    //---- Fresnel ---- 
    float cosTheta = dot(ray_dir, -world_normal);    
    float fresnel = pow(1.0 - cosTheta, 5.0);
    float kS = 0.04;       // specular fraction
    float kD = 1.0 - kS;   // diffuse fraction    
    kS += (kD * fresnel);  // more reflective at sharp angles
    kD = 1.0 - kS;
    //-----------------

    //---- Compose final color ----
    vec3 white     = vec3(1,1,1);
    vec3 metal     = mix(albedo.xyz, white, fresnel) * specular_light;   
    vec3 non_metal = mix(albedo.xyz * diffuse_light, specular_light, kS);
    payload.color  = mix(non_metal, metal, metalness) + emission.xyz;   
    //-----------------------------
    
    
    
    payload.distance = gl_HitTEXT;    
    //payload.hitpos   = world_hitpos;
    
    //payload.color = world_hitpos;
    //payload.color = normap.rgb;
    //payload.color = albedo.rgb * illum + emission.rgb;
    //payload.color = illum;
    
    //payload.color = specular_light;
    //payload.color = diffuse_light;
    //payload.color = metal;
    //payload.color = dielectric;
    //payload.color = vec3(fresnel,0,0);
    //payload.color = vec3(specular_light * kS);
    
    //payload.color += emission.xyz;
    
    //payload.color = vec3(cosTheta);
    //payload.color = vec3(fresnel);
    
    //payload.color = light.color.xyz;
    
    
    //float dot = dot(ray_dir, world_normal);    
    //if(dot>0) payload.color = vec3(0,0,0);
}
