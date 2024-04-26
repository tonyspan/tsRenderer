#version 450 core

layout (location = 0) in vec2 inTexCoord;

layout (location = 0) out vec4 outColor;

layout(binding = 1) uniform sampler2D uTexture;

layout(push_constant) uniform PC
{
    layout(offset = 64) float Opacity;
} constants;

void main()
{
    outColor = vec4(texture(uTexture, inTexCoord).rgb, constants.Opacity);
}
