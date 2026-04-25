#version 450
layout(set=0, binding=0, std140) uniform Frame {
    vec4 a;
    vec4 b;
} frame;
layout(location=0) out vec4 outColor;
void main() { outColor = frame.a + frame.b; }
