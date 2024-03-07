#pragma once

#include "Base.h"

#include "Vertex.h"

#include <string_view>
#include <span>

class Buffer;

class Mesh
{
public:
	static Ref<Mesh> Create(const std::string_view file);
	static Ref<Mesh> Create(const std::span<Vertex> vertices, const std::span<uint32_t> indices);
	static Ref<Mesh> Create(const Ref<Buffer>& vb, const Ref<Buffer>& ib);

	Mesh(const std::span<Vertex> vertices, const std::span<uint32_t> indices);
	Mesh(const Ref<Buffer>& vb, const Ref<Buffer>& ib);
	~Mesh();

	void SetName(const std::string_view name);

	const Buffer& GetVertexBuffer() const;
	const Buffer& GetIndexBuffer() const;

	uint32_t GetIndexCount() const;
private:
	std::string m_Name;

	Ref<Buffer> m_VertexBuffer;
	Ref<Buffer> m_IndexBuffer;

	// TODO: Temporary, probably IndexBuffer class is needed
	uint32_t m_Count = 0;
};