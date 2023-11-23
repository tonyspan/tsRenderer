#pragma once

#include "Base.h"

#include "VK.h"

#include "Enums.h"

#include <string_view>
#include <vector>

struct TextureDescription
{
	uint32_t Width;
	uint32_t Height;

	uint32_t MipLevels;
	Format Format;

	TextureDescription();
};

// TODO: Rework Texture, Texture2D, TextureCube classes

class Texture
{
public:
	Texture(TextureType type);
	virtual ~Texture() = default;

	TextureType GetType() const;
private:
	TextureType m_Type;
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

	TextureDescription m_Description;
};

class TextureCube : public Texture
{
public:
	// For Cubemaps
	static Ref<TextureCube> Create(const std::vector<std::string_view>& paths);

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

	TextureDescription m_Description;
};