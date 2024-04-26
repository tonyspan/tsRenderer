#include "Layout.h"

#include "Base.h"

#include "Log.h"

const std::vector<LayoutElement>& Layout::GetElements() const
{
	return m_Elements;
}

uint32_t Layout::GetStride() const
{
	ASSERT(m_Stride != 0, "");

	return m_Stride;
}

void Layout::Add(const std::string& name, Format format, uint32_t size)
{
	m_Elements.emplace_back(LayoutElement{ .Name = name, .Format = format, .Offset = m_Stride });

	m_Stride += size;
}