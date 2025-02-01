#version 450

// Note: Binding 0 is the UBO from the vertex shader.

layout (input_attachment_index = 0, set = 0, binding = 0) uniform subpassInput inputColor;
//layout (input_attachment_index = 1, set = 0, binding = 1) uniform subpassInput inputDepth;

layout(location = 0) out vec4 outColor;

void main() {      
    vec4 inColor = subpassLoad(inputColor);  // read from previous subpass
    outColor = inColor;
}
