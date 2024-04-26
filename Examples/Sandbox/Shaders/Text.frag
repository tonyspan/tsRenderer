#version 450

layout(location = 0) in vec2 inTexCoord;
layout(location = 0) out vec4 outColor;

layout(set = 0, binding = 1) uniform sampler2D uTexture;

void main()
{
    vec4 Tx = texture(uTexture, inTexCoord);
    outColor = ((vec4(1.0) * Tx.x) + (vec4(0.0, 0.0, 0.0, 1.0) * Tx.y)) + (vec4(0.0) * Tx.z);
}