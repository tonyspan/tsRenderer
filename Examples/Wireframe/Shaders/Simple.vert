#version 450

layout (location = 0) in vec3 inPosition;
layout (location = 1) in vec3 inNormal;
layout (location = 2) in vec2 inTexCoord;
layout (location = 3) in vec4 inColor;

layout (location = 0) out vec4 outColor;

layout (binding = 0) uniform UBO
{
    mat4 Model;
    mat4 View;
    mat4 Projection;
} ubo;

void main()
{
    outColor = inColor;
    gl_Position = ubo.Projection * ubo.View * ubo.Model * vec4(inPosition, 1.0);
}