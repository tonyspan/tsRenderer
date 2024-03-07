#pragma once

#include "Base.h"

#include <array>
#include <string_view>

class Mesh;
class TextureCube;

class Skybox
{
public:
	static Ref<Skybox> Create(const std::string_view meshPath, const std::array<std::string_view, 6>& paths);

	Skybox() = delete;

	Skybox(const std::string_view meshPath, const std::array<std::string_view, 6>& paths);
	~Skybox();

	const Mesh& GetMesh() const;
	const TextureCube& GetTexture() const;
private:
	Ref<Mesh> m_Mesh;
	Ref<TextureCube> m_Texture;
};