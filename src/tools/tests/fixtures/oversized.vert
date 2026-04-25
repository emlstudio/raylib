#version 450
layout(set=0, binding=0, std140) uniform U0 { float x; } u0;
layout(set=0, binding=1, std140) uniform U1 { float x; } u1;
layout(set=0, binding=2, std140) uniform U2 { float x; } u2;
layout(set=0, binding=3, std140) uniform U3 { float x; } u3;
layout(set=1, binding=0, std140) uniform U4 { float x; } u4;
layout(set=1, binding=1, std140) uniform U5 { float x; } u5;
layout(set=1, binding=2, std140) uniform U6 { float x; } u6;
layout(set=1, binding=3, std140) uniform U7 { float x; } u7;
layout(set=2, binding=0, std140) uniform U8 { float x; } u8;
layout(location=0) in vec3 inPos;
void main() { gl_Position = vec4(inPos, 1.0) * (u0.x + u1.x + u2.x + u3.x + u4.x + u5.x + u6.x + u7.x + u8.x); }
