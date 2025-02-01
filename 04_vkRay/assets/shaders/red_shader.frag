#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_GOOGLE_include_directive    : enable
#include "bindings.glsl"

//layout(binding = 2) uniform sampler2D image;

layout(location = 0) in  vec2 TC;
layout(location = 0) out vec4 outColor;

void main() {
    //vec4 image = texture(image, TC);
    //outColor = image;
//    outColor = vec4(1,0,0,1);


    vec4 color = model.material.albedo;
    vec4 image = texture(tex_albedo, TC);
    outColor = image * color;

}
