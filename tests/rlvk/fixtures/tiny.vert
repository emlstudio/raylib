#version 450

layout(location=0) in vec3 inPos;

layout(set=1, binding=0, std140) uniform Frame {
    float time;
} frame;

void main()
{
    gl_Position = vec4(inPos * frame.time, 1.0);
}
