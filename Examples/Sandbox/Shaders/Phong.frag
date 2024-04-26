#version 450

layout(location = 1) in vec3 inNormal;
layout(location = 0) in vec3 inPosition;
layout(location = 2) in vec2 inTexCoord;

layout(location = 0) out vec4 outColor;

layout(set = 0, binding = 2) uniform GUBO
{
    vec3 LightDirection;
    vec4 LightColor;
    vec3 CameraPosition;
} gubo;

layout(set = 0, binding = 1) uniform sampler2D uTexture;

void main()
{
    vec3 Norm = normalize(inNormal);
    vec3 EyeDir = normalize(gubo.CameraPosition - inPosition);
    vec3 Diffuse = texture(uTexture, inTexCoord).xyz * ((max(dot(Norm, gubo.LightDirection), 0.0) * 0.89999997615814208984375) + 0.100000001490116119384765625);
    vec3 Specular = vec3(pow(max(dot(EyeDir, -reflect(gubo.LightDirection, Norm)), 0.0), 64.0));
    outColor = vec4((Diffuse + Specular) * gubo.LightColor.xyz, 1.0);
}