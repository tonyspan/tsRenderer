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
	glm::vec3 Position;
	glm::vec3 Normal;
	glm::vec2 TexCoord;
};