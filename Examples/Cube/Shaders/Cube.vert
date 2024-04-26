#version 450 core

layout (location = 0) in vec3 inPosition;
layout (location = 1) in vec3 inNormal;
layout (location = 2) in vec2 inTexCoord;
layout (location = 3) in vec4 inColor;

layout (location = 0) out vec2 outTexCoord;

layout (binding = 0) uniform UBO
{
    mat4 View;
    mat4 Projection;
} ubo;

layout(push_constant) uniform PC
{
    mat4 Model;
} constants;

void main()
{
    outTexCoord = inTexCoord;
    gl_Position = ubo.Projection * ubo.View * constants.Model * vec4(inPosition, 1.0);
}
