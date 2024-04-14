#pragma once

#include "Base.h"

#include "Vertex.h"

#include <string_view>
#include <span>

class GBuffer;

class Mesh
{
public:
	static Ref<Mesh> Create(const std::string_view file);
	static Ref<Mesh> Create(const std::span<Vertex> vertices, const std::span<uint32_t> indices);
	static Ref<Mesh> Create(const Ref<GBuffer>& vb, const Ref<GBuffer>& ib);

	Mesh(const std::span<Vertex> vertices, const std::span<uint32_t> indices);
	Mesh(const Ref<GBuffer>& vb, const Ref<GBuffer>& ib);
	~Mesh();

	void SetName(const std::string_view name);

	const GBuffer& GetVertexBuffer() const;
	const GBuffer& GetIndexBuffer() const;

	uint32_t GetIndexCount() const;
private:
	std::string m_Name;

	Ref<GBuffer> m_VertexBuffer;
	Ref<GBuffer> m_IndexBuffer;
};