#pragma once

#include "VK.h"

#include <glm/glm.hpp>

#include <array>

struct Vertex
{
	glm::vec3 Position;
	glm::vec3 Normal;
	glm::vec2 TexCoord;
	glm::vec3 Color;

	static VkVertexInputBindingDescription GetBindingDescription();

	static std::array<VkVertexInputAttributeDescription, 4> GetAttributeDescriptions();
};