#version 450
layout(location=0) in vec2 fragTexCoord;
layout(location=1) in vec4 fragColor;

layout(set=0, binding=0) uniform sampler2D texture0;

layout(push_constant) uniform PC {
    mat4 mvp;
    vec4 colDiffuse;
} pc;

layout(location=0) out vec4 finalColor;

void main() {
    finalColor = texture(texture0, fragTexCoord) * fragColor * pc.colDiffuse;
}
