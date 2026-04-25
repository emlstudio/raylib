#version 450
layout(set=1, binding=0) uniform sampler2D tex0;
layout(set=1, binding=1) uniform sampler2D tex1;
layout(location=0) in vec2 uv;
layout(location=0) out vec4 outColor;
void main() { outColor = texture(tex0, uv) * texture(tex1, uv); }
