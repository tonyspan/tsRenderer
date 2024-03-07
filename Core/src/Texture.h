#pragma once

#include "Base.h"

#include "VK.h"

#include "Enums.h"

#include <string_view>
#include <array>

struct TextureDescription
{
	uint32_t Width = 0;
	uint32_t Height = 0;

	bool GenerateMipLevels = true;
	Format Format = Format::UNDEFINED;
};

// TODO: Rework Texture, Texture2D, TextureCube classes

class Texture
{
public:
	Texture(TextureType type, const TextureDescription& desc);
	virtual ~Texture() = default;

	TextureType GetType() const;
	uint32_t GenerateMips(uint32_t width, uint32_t height);

	const TextureDescription& GetDescription() const;
private:
	TextureType m_Type;
	TextureDescription m_Description;
};

class Image2D;
class Sampler;

class Texture2D : public Texture
{
public:
	static Ref<Texture2D> Create(const std::string_view path);

	Texture2D(void** data, const TextureDescription& desc);
	~Texture2D();

	const Image2D& GetImage() const;
	const Sampler& GetSampler() const;
private:
	void CreateTexture(void** data);
	void CreateSampler();
private:
	Ref<Image2D> m_Image;
	Ref<Sampler> m_Sampler;
};

class TextureCube : public Texture
{
public:
	// For Cubemaps
	static Ref<TextureCube> Create(const std::array<std::string_view, 6>& paths);

	TextureCube(void** data, const TextureDescription& desc);
	~TextureCube();

	const Image2D& GetImage() const;
	const Sampler& GetSampler() const;
private:
	void CreateCube(void** data);
	void CreateSampler();
private:
	Ref<Image2D> m_Image;
	Ref<Sampler> m_Sampler;
};