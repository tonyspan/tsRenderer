#version 450

layout(location = 0) in vec3 inPosition;
layout(location = 2) in vec2 inTexCoord;
layout(location = 1) in vec3 inNormal;

layout(location = 0) out vec2 outTexCoord;

layout(set = 0, binding = 0) uniform UBO
{
    mat4 mvpMat;
    mat4 mMat;
    mat4 nMat;
} ubo;

void main()
{
    gl_Position = vec4(inPosition, 1.0);
    outTexCoord = inTexCoord;
}