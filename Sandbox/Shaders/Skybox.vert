#version 450

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec2 inTexCoord;

layout(location = 0) out vec3 outTexCoord;

layout(set = 0, binding = 0) uniform UBO
{
    mat4 MVP;
    mat4 Model;
    mat4 Normal; // unused
} ubo;

void main()
{
    outTexCoord = inPosition;
    vec4 pos = ubo.MVP * vec4(inPosition, 1.0);
    gl_Position = pos.xyww;
}
