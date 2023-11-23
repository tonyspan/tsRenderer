#pragma once

#include "Base.h"

#include <vector>
#include <string_view>

class Mesh;
class TextureCube;

class Skybox
{
public:
	static Ref<Skybox> Create(const std::string_view meshPath, const std::vector<std::string_view>& paths);

	Skybox() = delete;

	Skybox(const std::string_view meshPath, const std::vector<std::string_view>& paths);
	~Skybox();

	const Mesh& GetMesh() const;
	const TextureCube& GetTexture() const;
private:
	Ref<Mesh> m_Mesh;
	Ref<TextureCube> m_Texture;
};