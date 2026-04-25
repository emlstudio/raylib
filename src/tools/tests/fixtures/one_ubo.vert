#version 450
layout(set=0, binding=0, std140) uniform Frame {
    mat4 view;
    mat4 proj;
    vec4 time;
} frame;
layout(location=0) in vec3 inPos;
void main() { gl_Position = frame.proj * frame.view * vec4(inPos, 1.0); }
