#pragma once

#include "Base.h"

#include "Vertex.h"

#include <string_view>
#include <vector>

class Buffer;

class Mesh
{
public:
	static Ref<Mesh> Create(const std::string_view file);
	static Ref<Mesh> Create(const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices);

	Mesh(const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices);
	~Mesh();

	void SetName(const std::string_view name);

	const Buffer& GetVertexBuffer() const;
	const Buffer& GetIndexBuffer() const;

	uint32_t GetIndexCount() const;
private:
	std::string m_Name;

	Ref<Buffer> m_VertexBuffer;
	Ref<Buffer> m_IndexBuffer;

	std::vector<Vertex> m_Vertices;
	std::vector<uint32_t> m_Indices;
};