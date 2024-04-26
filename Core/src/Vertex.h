#pragma once

#include "VK.h"

#include <glm/glm.hpp>

struct SimpleVertex
{
	glm::vec3 Position;
	glm::vec3 Color;
};

struct Vertex
{
	glm::vec3 Position = glm::vec3(0.0f);
	glm::vec3 Normal = glm::vec3(0.0f);
	glm::vec2 TexCoord = glm::vec2(0.0f);
	glm::vec4 Color = glm::vec4(0.0f);
};