#version 450
layout(set=1, binding=0, std140) uniform Material {
    vec4 albedo;
    float roughness;
} mat;
layout(location=0) out vec4 outColor;
void main() { outColor = mat.albedo * mat.roughness; }
