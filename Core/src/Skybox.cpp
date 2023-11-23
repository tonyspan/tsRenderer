#include "Skybox.h"

#include "Mesh.h"
#include "Texture.h"

Ref<Skybox> Skybox::Create(const std::string_view meshPath, const std::vector<std::string_view>& paths)
{
	return CreateRef<Skybox>(meshPath, paths);
}

Skybox::Skybox(const std::string_view meshPath, const std::vector<std::string_view>& paths)
	: m_Mesh(Mesh::Create(meshPath)), m_Texture(TextureCube::Create(paths))
{
}

Skybox::~Skybox()
{
	m_Mesh.reset();
	m_Texture.reset();
}

const Mesh& Skybox::GetMesh() const
{
	return *m_Mesh;
}

const TextureCube& Skybox::GetTexture() const
{
	return *m_Texture;
}
