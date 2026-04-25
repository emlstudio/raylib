#version 450
layout(set=0, binding=0, std140) uniform Padding {
    vec3 a;     // offset 0
    float b;    // offset 12 (packs into the same vec4 as a)
    vec3 c;     // offset 16
    vec2 d;     // offset 32 (next vec4 boundary; 28 not allowed because vec2 wraps a vec4 boundary)
} pad;
layout(location=0) in vec3 inPos;
void main() { gl_Position = vec4(pad.a + inPos, pad.b) + vec4(pad.c, pad.d); }
