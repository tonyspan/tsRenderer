#include "Mesh.h"

#include "Buffer.h"

#include "Log.h"

#include <vulkan/vulkan.h>

#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>

static bool LoadFromFile(const std::string_view file, std::vector<Vertex>& vertices, std::vector<uint32_t>& indices)
{
	tinyobj::attrib_t attrib;
	std::vector<tinyobj::shape_t> shapes;
	std::vector<tinyobj::material_t> materials;

	std::string warn, err;

	bool sucess = tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, file.data());
	if (!sucess)
	{
		LOG("%s: %s", file.data(), (warn + err).data());
		return false;
	}

	for (const auto& shape : shapes)
	{
		for (const auto& index : shape.mesh.indices)
		{
			Vertex vertex = {};

			vertex.Position = {
				attrib.vertices[3 * index.vertex_index + 0],
				attrib.vertices[3 * index.vertex_index + 1],
				attrib.vertices[3 * index.vertex_index + 2]
			};

			vertex.TexCoord = {
				attrib.texcoords[2 * index.texcoord_index + 0],
				1 - attrib.texcoords[2 * index.texcoord_index + 1]
			};

			vertex.Normal = {
				attrib.normals[3 * index.normal_index + 0],
				attrib.normals[3 * index.normal_index + 1],
				attrib.normals[3 * index.normal_index + 2]
			};

			vertices.emplace_back(vertex);

			indices.emplace_back(vertices.size() - 1);
		}
	}

	return true;
}

Ref<Mesh> Mesh::Create(const std::string_view file)
{
	std::vector<Vertex> vertices;
	std::vector<uint32_t> indices;

	if (LoadFromFile(file, vertices, indices))
		return CreateRef<Mesh>(vertices, indices);

	return nullptr;
}

Ref<Mesh> Mesh::Create(const std::span<Vertex> vertices, const std::span<uint32_t> indices)
{
	if (vertices.empty() || indices.empty())
		return nullptr;

	return CreateRef<Mesh>(vertices, indices);
}

Ref<Mesh> Mesh::Create(const Ref<Buffer>& vb, const Ref<Buffer>& ib)
{
	if (nullptr == vb || nullptr == ib)
		return nullptr;

	return CreateRef<Mesh>(vb, ib);
}

Mesh::Mesh(const std::span<Vertex> vertices, const std::span<uint32_t> indices)
	: m_Count(static_cast<uint32_t>(indices.size()))
{
	const uint64_t verticesSize = static_cast<uint64_t>(sizeof(Vertex) * vertices.size());
	const uint64_t indicesSize = static_cast<uint64_t>(sizeof(uint32_t) * indices.size());

	m_VertexBuffer = Buffer::CreateVertex(verticesSize);
	m_VertexBuffer->SetData(static_cast<const void*>(vertices.data()), verticesSize);

	m_IndexBuffer = Buffer::CreateIndex(indicesSize);
	m_IndexBuffer->SetData(static_cast<const void*>(indices.data()), indicesSize);
}

Mesh::Mesh(const Ref<Buffer>& vb, const Ref<Buffer>& ib)
	: m_VertexBuffer(vb), m_IndexBuffer(ib)
{
	m_Count = ib->GetDescription().Size / sizeof(uint32_t);
}

Mesh::~Mesh()
{
	m_VertexBuffer.reset();
	m_IndexBuffer.reset();
}

void Mesh::SetName(const std::string_view name)
{
	m_Name = name;
}

const Buffer& Mesh::GetVertexBuffer() const
{
	return *m_VertexBuffer;
}

const Buffer& Mesh::GetIndexBuffer() const
{
	return *m_IndexBuffer;
}

uint32_t Mesh::GetIndexCount() const
{
	return m_Count;
}

