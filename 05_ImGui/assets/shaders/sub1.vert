#version 450

void main() {
    vec2 verts[3] = {{-1.0,-4.0}, {-1.0,1.0}, {4.0,1.0}};
    vec2 v = verts[gl_VertexIndex];
    gl_Position = vec4(v, 0.0f, 1.0f);
}
