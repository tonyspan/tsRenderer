#include "Texture.h"

#include "Context.h"
#include "Device.h"
#include "Image.h"
#include "Buffer.h"
#include "GBuffer.h"
#include "Sampler.h"

#include "Log.h"

#include <vulkan/vulkan.h>

#include <glm/glm.hpp>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include <span>

static uint8_t* LoadFromFile(const std::string_view filename, uint32_t& width, uint32_t& height, uint32_t& channels)
{
	if (filename.empty())
		return nullptr;

	uint8_t* data = nullptr;

	data = (uint8_t*)stbi_load(filename.data(), (int*)&width, (int*)&height, (int*)&channels, STBI_rgb_alpha);

	if (!data)
		LOG("Failed to load texture image, %s not found", filename.data());

	return data;
}

template<typename T>
static Ref<T> TryCreate(const std::span<const std::string_view> paths)
{
	if (paths.empty())
		return nullptr;

	const uint32_t imageCount = static_cast<uint32_t>(paths.size());

	uint32_t width = 0, height = 0, channels = 0;
	int32_t curWidth = -1, curHeight = -1, curChannels = -1;

	for (uint32_t i = 0; i < imageCount; i++)
	{
		const auto& path = paths[i];

		// Don't care about the actual image just yet, only for the image metadata
		uint8_t* data = LoadFromFile(path, width, height, channels);

		stbi_image_free(data);

		if (i == 0)
		{
			curWidth = width;
			curHeight = height;
			curChannels = channels;
		}
		else
		{
			ASSERT(!((curWidth != width) || (curHeight != height) || (curChannels != channels)), "All images must have the same size");
		}
	}

	ASSERT((width != 0) || (height != 0) || (channels != 0));

	// R8G8B8 isn't supported by Vulkan?
	if (3 == channels)
		channels = 4;

	TextureDescription desc;

	desc.Width = width;
	desc.Height = height;
	desc.ImageCount = imageCount;
	desc.Format = FormatBytesPerPixel(channels);

	const uint64_t imageSize = width * height * channels;
	const uint64_t totalImagesSize = imageSize * imageCount;

	Buffer buffer;
	buffer.Allocate(totalImagesSize);

	for (uint32_t i = 0; i < imageCount; i++)
	{
		const auto& path = paths[i];

		uint8_t* data = LoadFromFile(path, width, height, channels);

		if (data)
		{
			buffer.Write(data, imageSize, imageSize * i);

			stbi_image_free(data);

			LOG("Loaded %s, size: (%ix%i), channels: %i", path.data(), width, height, channels);
		}
	}

	Ref<T> texture = T::Create(desc, buffer);

	buffer.Release();

	return texture;
}

Ref<Texture> Texture::Create(const std::string_view path)
{
	std::array<std::string_view, 1> paths = { path };

	return TryCreate<Texture2D>(paths);
}

Ref<Texture> Texture::Create(const std::array<std::string_view, s_MaxImageCount>& paths)
{
	return TryCreate<TextureCube>(paths);
}

template<>
Ref<Texture> Texture::White<TextureType::TEXTURE2D>()
{
	static constexpr uint32_t s_WhiteTextureData = 0xffffffff;

	TextureDescription desc;

	desc.Width = 1;
	desc.Height = 1;
	desc.ImageCount = 1;
	desc.Format = Format::RGBA_8_SRGB;

	return Texture2D::Create(desc, Buffer((void*)&s_WhiteTextureData, sizeof(uint32_t)));
}

template<>
Ref<Texture> Texture::White<TextureType::CUBE>()
{
	static constexpr uint32_t s_WhiteTextureData[6] = { 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff };

	TextureDescription desc;

	desc.Width = 1;
	desc.Height = 1;
	desc.ImageCount = (uint32_t)std::size(s_WhiteTextureData);
	desc.Format = Format::RGBA_8_SRGB;

	return TextureCube::Create(desc, Buffer((void*)&s_WhiteTextureData, sizeof(s_WhiteTextureData)));
}

Texture::Texture(TextureType type, const TextureDescription& desc)
	: m_Type(type), m_Description(desc)
{
}

Texture::~Texture()
{
	m_Sampler.reset();
	m_Image.reset();
}

TextureType Texture::GetType() const
{
	return m_Type;
}

uint32_t Texture::GenerateMips()
{
	return static_cast<uint32_t>(glm::floor(std::log2(glm::max(m_Description.Width, m_Description.Height)))) + 1;
}

const TextureDescription& Texture::GetDescription() const
{
	return m_Description;
}

void Texture::CreateTexture(const Buffer& buffer)
{
	ASSERT(buffer);

	const uint32_t width = m_Description.Width;
	const uint32_t height = m_Description.Height;
	const uint32_t imageCount = m_Description.ImageCount;

	ASSERT(width != 0 && height != 0);
	ASSERT(imageCount > 0 && imageCount <= s_MaxImageCount);

	const uint64_t imageSize = width * height * FormatBytesPerPixel(m_Description.Format);
	const uint64_t totalImagesSize = imageSize * imageCount;
	ASSERT(buffer.GetSize() == totalImagesSize);

	Scope<GBuffer> stagingBuffer = GBuffer::CreateStaging(buffer.GetSize());

	ImageDescription desc;

	desc.Width = width;
	desc.Height = height;
	desc.MipLevels = m_Description.GenerateMipLevels ? GenerateMips() : 1;
	desc.ImageCount = imageCount;
	desc.MSAAnumSamples = 1;
	desc.Format = m_Description.Format;
	desc.ImageUsage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
	desc.Properties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
	desc.ImageAspectFlags = VK_IMAGE_ASPECT_COLOR_BIT;

	if (s_MaxImageCount == imageCount)
	{
		desc.ImageCreateFlags = VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;
		desc.ViewType = VK_IMAGE_VIEW_TYPE_CUBE;

		// Technically the else case works for this one too
		// Just a different approach
		for (uint32_t i = 0; i < imageCount; i++)
		{
			const uint64_t offset = imageSize * i;
			stagingBuffer->SetData(&buffer[offset], imageSize, offset);
		}
	}
	else
	{
		desc.ImageCreateFlags = 0;
		desc.ViewType = VK_IMAGE_VIEW_TYPE_2D;

		stagingBuffer->SetData(buffer.As<uint8_t>());
	}

	m_Image = Image2D::Create(desc);

	m_Image->TransitionImageLayout(VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
	m_Image->CopyFrom(*stagingBuffer);
}

void Texture::CreateSampler()
{
	SamplerDescription desc;

	desc.MipLevels = Texture::GetDescription().GenerateMipLevels ? GenerateMips() : 1;
	desc.MagFilter = Filter::NEAREST;
	desc.MinFilter = Filter::NEAREST;

	m_Sampler = Sampler::Create(desc);
}

const Image2D& Texture::GetImage() const
{
	ASSERT(m_Image);

	return *m_Image;
}

const Ref<Sampler> Texture::GetSampler() const
{
	return m_Sampler;
}

Ref<Texture2D> Texture2D::Create(const TextureDescription& desc, const Buffer& buffer)
{
	return CreateRef<Texture2D>(desc, buffer);
}

Texture2D::Texture2D(const TextureDescription& desc, const Buffer& buffer)
	: Texture(TextureType::TEXTURE2D, desc)
{
	CreateTexture(buffer);

	if (desc.CreateSampler)
		CreateSampler();
}

Ref<TextureCube> TextureCube::Create(const TextureDescription& desc, const Buffer& buffer)
{
	return CreateRef<TextureCube>(desc, buffer);
}

TextureCube::TextureCube(const TextureDescription& desc, const Buffer& buffer)
	: Texture(TextureType::CUBE, desc)
{
	CreateTexture(buffer);

	if (desc.CreateSampler)
		CreateSampler();
}
