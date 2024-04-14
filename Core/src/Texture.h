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
	uint32_t ImageCount = 0;

	bool GenerateMipLevels = true;
	bool CreateSampler = true;
	Format Format = Format::UNDEFINED;
};

class Image2D;
class Sampler;
class Buffer;

// TODO: Rework Texture, Texture2D, TextureCube classes <- Better now?

class Texture
{
public:
	static constexpr size_t s_MaxImageCount = 6;

	static Ref<Texture> Create(const std::string_view path);
	static Ref<Texture> Create(const std::array<std::string_view, s_MaxImageCount>& paths);

	template<TextureType Type>
	static Ref<Texture> White();

	Texture(TextureType type, const TextureDescription& desc);
	virtual ~Texture();

	TextureType GetType() const;
	uint32_t GenerateMips();

	const Image2D& GetImage() const;
	const Ref<Sampler> GetSampler() const;

	const TextureDescription& GetDescription() const;
protected:
	void CreateTexture(const Buffer& buffer);
	void CreateSampler();
private:
	TextureDescription m_Description;
	TextureType m_Type;

	Ref<Image2D> m_Image;
	Ref<Sampler> m_Sampler;
};

class Texture2D : public Texture
{
public:
	static Ref<Texture2D> Create(const TextureDescription& desc, const Buffer& buffer);

	Texture2D(const TextureDescription& desc, const Buffer& buffer);
	~Texture2D() = default;
};

class TextureCube : public Texture
{
public:
	static Ref<TextureCube> Create(const TextureDescription& desc, const Buffer& buffer);

	TextureCube(const TextureDescription& desc, const Buffer& buffer);
	~TextureCube() = default;
};