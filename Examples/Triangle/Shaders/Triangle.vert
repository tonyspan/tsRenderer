#version 450

layout (location = 0) in vec3 inPosition;
layout (location = 1) in vec3 inColor;

layout (location = 0) out vec3 outColor;

//layout (binding = 0) uniform UBO 
//{
//	mat4 projectionMatrix;
//	mat4 modelMatrix;
//	mat4 viewMatrix;
//} ubo;

void main()
{
	outColor = inColor;
	gl_Position = /* ubo.projectionMatrix * ubo.viewMatrix * ubo.modelMatrix * */ vec4(inPosition.xyz, 1.0);
}