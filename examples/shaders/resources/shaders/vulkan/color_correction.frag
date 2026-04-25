#version 450

// Vulkan port of resources/shaders/glsl330/color_correction.fs.
// Differences from the GLSL 330 source:
//  - file-scope uniform floats (contrast, saturation, brightness) move into
//    a UBO at set=1 binding=0 (Vulkan has no default uniform block).
//  - colDiffuse comes from the push-constant block instead of a uniform,
//    matching raylib's rlvk default-shader contract.

layout(location=0) in vec2 fragTexCoord;
layout(location=1) in vec4 fragColor;

layout(set=0, binding=0) uniform sampler2D texture0;

layout(set=1, binding=0, std140) uniform Correction {
    float contrast;
    float saturation;
    float brightness;
} c;

layout(push_constant) uniform PC {
    mat4 mvp;
    vec4 colDiffuse;
} pc;

layout(location=0) out vec4 finalColor;

void main()
{
    vec4 texel = texture(texture0, fragTexCoord);

    // Apply contrast
    texel.rgb = (texel.rgb - 0.5)*(c.contrast/100.0 + 1.0) + 0.5;

    // Apply brightness
    texel.rgb = texel.rgb + c.brightness/100.0;

    // Apply saturation
    float intensity = dot(texel.rgb, vec3(0.299, 0.587, 0.114));
    texel.rgb = (texel.rgb - intensity)*c.saturation/100.0 + texel.rgb;

    finalColor = texel*pc.colDiffuse*fragColor;
}
