#version 450
layout(set=0, binding=0, std140) uniform Frame {
    mat4 mvp;
    float time;
} frame;
layout(location=0) in vec3 inPos;
void main() { gl_Position = frame.mvp * vec4(inPos, 1.0) * frame.time; }
