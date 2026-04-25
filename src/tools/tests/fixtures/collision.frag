#version 450
layout(set=1, binding=0) uniform sampler2D tex0;
layout(location=0) in vec2 dummyUV;
layout(location=0) out vec4 outColor;
void main() { outColor = texture(tex0, dummyUV); }
