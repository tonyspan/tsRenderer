#version 450 core

layout (location = 0) in vec3 inPosition;
layout (location = 1) in vec2 inTexCoord;

layout (location = 0) out vec2 outTexCoord;

layout (binding = 0) uniform UBO 
{
    mat4 MVP;
} ubo;

void main()
{
    outTexCoord = inTexCoord;
    gl_Position = ubo.MVP * vec4(inPosition, 1.0);
}
