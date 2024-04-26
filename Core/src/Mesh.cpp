#include "Mesh.h"

#include "GBuffer.h"

#include "Log.h"

#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>

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

static Ref<Mesh> CreateCube()
{
	Vertex vertices[] = {
		{.Position = { -0.5f, -0.5f, -0.5f }, .TexCoord = { 0.0f, 0.0f } },
		{.Position = {  0.5f,  0.5f, -0.5f }, .TexCoord = { 1.0f, 1.0f } },
		{.Position = {  0.5f, -0.5f, -0.5f }, .TexCoord = { 1.0f, 0.0f } },
		{.Position = {  0.5f,  0.5f, -0.5f }, .TexCoord = { 1.0f, 1.0f } },
		{.Position = { -0.5f, -0.5f, -0.5f }, .TexCoord = { 0.0f, 0.0f } },
		{.Position = { -0.5f,  0.5f, -0.5f }, .TexCoord = { 0.0f, 1.0f } },

		{.Position = { -0.5f, -0.5f,  0.5f }, .TexCoord = { 0.0f, 0.0f } },
		{.Position = {  0.5f, -0.5f,  0.5f }, .TexCoord = { 1.0f, 0.0f } },
		{.Position = {  0.5f,  0.5f,  0.5f }, .TexCoord = { 1.0f, 1.0f } },
		{.Position = {  0.5f,  0.5f,  0.5f }, .TexCoord = { 1.0f, 1.0f } },
		{.Position = { -0.5f,  0.5f,  0.5f }, .TexCoord = { 0.0f, 1.0f } },
		{.Position = { -0.5f, -0.5f,  0.5f }, .TexCoord = { 0.0f, 0.0f } },

		{.Position = { -0.5f,  0.5f,  0.5f }, .TexCoord = { 1.0f, 0.0f } },
		{.Position = { -0.5f,  0.5f, -0.5f }, .TexCoord = { 1.0f, 1.0f } },
		{.Position = { -0.5f, -0.5f, -0.5f }, .TexCoord = { 0.0f, 1.0f } },
		{.Position = { -0.5f, -0.5f, -0.5f }, .TexCoord = { 0.0f, 1.0f } },
		{.Position = { -0.5f, -0.5f,  0.5f }, .TexCoord = { 0.0f, 0.0f } },
		{.Position = { -0.5f,  0.5f,  0.5f }, .TexCoord = { 1.0f, 0.0f } },

		{.Position = { 0.5f,  0.5f,  0.5f }, .TexCoord = { 1.0f, 0.0f } },
		{.Position = { 0.5f, -0.5f, -0.5f }, .TexCoord = { 0.0f, 1.0f } },
		{.Position = { 0.5f,  0.5f, -0.5f }, .TexCoord = { 1.0f, 1.0f } },
		{.Position = { 0.5f, -0.5f, -0.5f }, .TexCoord = { 0.0f, 1.0f } },
		{.Position = { 0.5f,  0.5f,  0.5f }, .TexCoord = { 1.0f, 0.0f } },
		{.Position = { 0.5f, -0.5f,  0.5f }, .TexCoord = { 0.0f, 0.0f } },

		{.Position = { -0.5f, -0.5f, -0.5f }, .TexCoord = { 0.0f, 1.0f } },
		{.Position = {  0.5f, -0.5f, -0.5f }, .TexCoord = { 1.0f, 1.0f } },
		{.Position = {  0.5f, -0.5f,  0.5f }, .TexCoord = { 1.0f, 0.0f } },
		{.Position = {  0.5f, -0.5f,  0.5f }, .TexCoord = { 1.0f, 0.0f } },
		{.Position = { -0.5f, -0.5f,  0.5f }, .TexCoord = { 0.0f, 0.0f } },
		{.Position = { -0.5f, -0.5f, -0.5f }, .TexCoord = { 0.0f, 1.0f } },

		{.Position = { -0.5f,  0.5f, -0.5f }, .TexCoord = { 0.0f, 1.0f } },
		{.Position = {  0.5f,  0.5f,  0.5f }, .TexCoord = { 1.0f, 0.0f } },
		{.Position = {  0.5f,  0.5f, -0.5f }, .TexCoord = { 1.0f, 1.0f } },
		{.Position = {  0.5f,  0.5f,  0.5f }, .TexCoord = { 1.0f, 0.0f } },
		{.Position = { -0.5f,  0.5f, -0.5f }, .TexCoord = { 0.0f, 1.0f } },
		{.Position = { -0.5f,  0.5f,  0.5f }, .TexCoord = { 0.0f, 0.0f } }
	};

	for (auto& vertex : vertices)
	{
		vertex.Normal = glm::normalize(vertex.Position);
		vertex.Color = glm::vec4(1.0f);
	}

	uint32_t indices[] = {
		0, 1, 2,
		1, 2, 3,
		4, 5, 6,
		5, 6, 7,
		8, 9, 10,
		9, 10, 11,
		12, 13, 14,
		13, 14, 15,
		16, 17, 18,
		17, 18, 19,
		20, 21, 22,
		21, 22, 23
	};

	return Mesh::Create(vertices, indices);
}

static Ref<Mesh> CreateSphere(uint32_t rings = 32, uint32_t sectors = 32, float radius = 1.0f)
{
	constexpr float PI = glm::pi<float>();
	constexpr float PI_2 = PI / 2.0f;

	std::vector<Vertex> vertices;
	std::vector<uint32_t> indices;

	const float R = 1.0f / static_cast<float>(rings - 1);
	const float S = 1.0f / static_cast<float>(sectors - 1);

	for (uint32_t r = 0; r < rings; r++)
	{
		for (uint32_t s = 0; s < sectors; s++)
		{
			const float y = sin(-PI_2 + PI * r * R);
			const float x = cos(2 * PI * s * S) * sin(PI * r * R);
			const float z = sin(2 * PI * s * S) * sin(PI * r * R);

			Vertex vertex;
			vertex.Position = glm::vec3(x, y, z) * radius;
			vertex.Normal = glm::normalize(vertex.Position);

			vertices.push_back(vertex);
		}
	}

	for (uint32_t r = 0; r < rings - 1; r++)
	{
		for (uint32_t s = 0; s < sectors - 1; s++)
		{
			uint32_t currentIdx = r * sectors + s;
			uint32_t nextIdx = currentIdx + sectors;

			indices.push_back(currentIdx);
			indices.push_back(nextIdx);
			indices.push_back(currentIdx + 1);

			indices.push_back(nextIdx);
			indices.push_back(nextIdx + 1);
			indices.push_back(currentIdx + 1);
		}
	}

	return Mesh::Create(vertices, indices);
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

Ref<Mesh> Mesh::Create(const Ref<GBuffer>& vb, const Ref<GBuffer>& ib)
{
	if (nullptr == vb || nullptr == ib)
		return nullptr;

	return CreateRef<Mesh>(vb, ib);
}

Ref<Mesh> Mesh::Create(MeshPrimitiveType type)
{
	switch (type)
	{
	case MeshPrimitiveType::CUBE:
		return CreateCube();
	case MeshPrimitiveType::SPHERE:
		return CreateSphere();
	default:
		break;
	}

	ASSERT(false);

	return nullptr;
}

Mesh::Mesh(const std::span<Vertex> vertices, const std::span<uint32_t> indices)
{
	const uint64_t verticesSize = static_cast<uint64_t>(sizeof(Vertex) * vertices.size());
	const uint64_t indicesSize = static_cast<uint64_t>(sizeof(uint32_t) * indices.size());

	m_VertexBuffer = GBuffer::CreateVertex(verticesSize);
	m_VertexBuffer->SetData(static_cast<const void*>(vertices.data()), verticesSize);

	m_IndexBuffer = GBuffer::CreateIndex(indicesSize, static_cast<uint32_t>(indices.size()));
	m_IndexBuffer->SetData(static_cast<const void*>(indices.data()), indicesSize);
}

Mesh::Mesh(const Ref<GBuffer>& vb, const Ref<GBuffer>& ib)
	: m_VertexBuffer(vb), m_IndexBuffer(ib)
{
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

const GBuffer& Mesh::GetVertexBuffer() const
{
	ASSERT(m_VertexBuffer);

	return *m_VertexBuffer;
}

const GBuffer& Mesh::GetIndexBuffer() const
{
	ASSERT(m_IndexBuffer);

	return *m_IndexBuffer;
}

uint32_t Mesh::GetIndexCount() const
{
	ASSERT(m_IndexBuffer);

	return m_IndexBuffer->GetDescription().IndexCount;
}

