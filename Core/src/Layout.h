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
	uint32_t m_Stride = 0;
	std::vector<LayoutElement> m_Elements;
};

#define LAYOUT_SPECIALIZATION(TYPE, FORMAT) \
template <> \
inline void Layout::Add<TYPE>(const std::string& name) \
{ \
	Add(name, FORMAT, sizeof(TYPE)); \
}

LAYOUT_SPECIALIZATION(uint32_t, Format::R32_UINT)
LAYOUT_SPECIALIZATION(uint8_t, Format::R8_UINT)
LAYOUT_SPECIALIZATION(float, Format::R32_SFLOAT)
LAYOUT_SPECIALIZATION(glm::vec2, Format::RG_32_SFLOAT)
LAYOUT_SPECIALIZATION(glm::vec3, Format::RGB_32_SFLOAT)
LAYOUT_SPECIALIZATION(glm::vec4, Format::RGBA_32_SFLOAT)