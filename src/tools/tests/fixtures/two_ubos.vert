#version 450
layout(set=0, binding=0, std140) uniform Frame {
    mat4 view;
    mat4 proj;
} frame;
layout(set=1, binding=0, std140) uniform Material {
    vec4 albedo;
    float roughness;
} mat;
layout(location=0) in vec3 inPos;
void main() { gl_Position = frame.proj * frame.view * vec4(inPos * mat.roughness, 1.0) + mat.albedo; }
