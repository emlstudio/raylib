#version 450

// Pass-through 2D batch vertex shader (Vulkan port of raylib's default).
// Matches the BATCH_2D layout the rlvk runtime feeds: pos / uv / normal / color
// across four vertex bindings, with mat4 mvp + vec4 colDiffuse delivered via
// push constants.

layout(location=0) in vec3 vertexPosition;
layout(location=1) in vec2 vertexTexCoord;
layout(location=2) in vec3 vertexNormal;
layout(location=3) in vec4 vertexColor;

layout(push_constant) uniform PC {
    mat4 mvp;
    vec4 colDiffuse;
} pc;

layout(location=0) out vec2 fragTexCoord;
layout(location=1) out vec4 fragColor;

void main()
{
    fragTexCoord = vertexTexCoord;
    fragColor    = vertexColor;
    gl_Position  = pc.mvp * vec4(vertexPosition, 1.0);
}
