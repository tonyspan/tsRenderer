#pragma once

#include "Enums.h"

#include <glm/glm.hpp>

#include <string>
#include <vector>

// Other name(s) BufferLayout

struct LayoutElement
{
	std::string Name;
	Format Format = Format::UNDEFINED;
	uint32_t Offset = 0;
};

class Layout
{
public:
	Layout();

	template <typename T>
	void Add(const std::string& name)
	{
		static_assert(sizeof(T) == 0, "Unknown type");
	}

	const std::vector<LayoutElement>& GetElements() const;

	uint32_t GetStride() const;
private:
	void Add(const std::string& name, Format format, uint32_t size);
private:
	uint32_t m_Stride;
	std::vector<LayoutElement> m_Elements;

};

#pragma region Specialization

template <>
inline void Layout::Add<uint32_t>(const std::string& name)
{
	Add(name, Format::R32_UINT, sizeof(uint32_t));
}

template <>
inline void Layout::Add<uint8_t>(const std::string& name)
{
	Add(name, Format::R8_UINT, sizeof(uint8_t));
}

template <>
inline void Layout::Add<float>(const std::string& name)
{
	Add(name, Format::R32_SFLOAT, sizeof(float));
}

template <>
inline void Layout::Add<glm::vec2>(const std::string& name)
{
	Add(name, Format::RG_32_SFLOAT, sizeof(glm::vec2));
}

template <>
inline void Layout::Add<glm::vec3>(const std::string& name)
{
	Add(name, Format::RGB_32_SFLOAT, sizeof(glm::vec3));
}

template <>
inline void Layout::Add<glm::vec4>(const std::string& name)
{
	Add(name, Format::RGBA_32_SFLOAT, sizeof(glm::vec4));
}

#pragma endregion