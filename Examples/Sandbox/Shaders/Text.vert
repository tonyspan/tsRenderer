#version 450

layout (location = 0) in vec3 inPosition;
layout (location = 1) in vec3 inNormal;
layout (location = 2) in vec2 inTexCoord;
layout (location = 3) in vec4 inColor;

layout(location = 0) out vec2 outTexCoord;

layout(set = 0, binding = 0) uniform UBO
{
    mat4 MVP;
    mat4 Model;
    mat4 Normal;
} ubo;

void main()
{
    gl_Position = vec4(inPosition, 1.0);
    outTexCoord = inTexCoord;
}