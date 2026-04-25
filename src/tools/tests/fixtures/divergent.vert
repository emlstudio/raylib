#version 450
layout(set=0, binding=0, std140) uniform Frame {
    vec4 a;
} frame;
layout(location=0) in vec3 inPos;
void main() { gl_Position = vec4(inPos, 1.0) + frame.a; }
