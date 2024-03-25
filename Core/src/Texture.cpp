#include "Texture.h"

#include "Context.h"
#include "Device.h"
#include "Image.h"
#include "Buffer.h"
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
static Ref<T> TryCreate(const std::span<const std::string_view> paths, TextureDescription& desc)
{
	if (paths.empty())
		return nullptr;

	uint8_t** data = new uint8_t * [paths.size()]();

	uint32_t channels;
	int32_t curWidth = -1, curHeight = -1, curChannels = -1;

	uint32_t& width = desc.Width;
	uint32_t& height = desc.Height;
	for (size_t i = 0; i < paths.size(); i++)
	{
		const auto& path = paths[i];

		data[i] = LoadFromFile(path, width, height, channels);

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

		LOG("Loaded %s, size: (%ix%i), channels: %i", path.data(), width, height, channels);
	}

	auto imageFreeFunc = [&data, imageCount = paths.size()]()
		{
			for (uint32_t i = 0; i < imageCount; i++)
			{
				stbi_image_free(data[i]);
				data[i] = nullptr;
			}

			delete[] data;
			data = nullptr;
		};

	if (data)
	{
		Ref<T> texture = CreateRef<T>((void**)data, desc);

		imageFreeFunc();

		return texture;
	}

	imageFreeFunc();

	return nullptr;
}

Texture::Texture(TextureType type, const TextureDescription& desc)
	: m_Type(type), m_Description(desc)
{
}

TextureType Texture::GetType() const
{
	return m_Type;
}

uint32_t Texture::GenerateMips()
{
	const uint32_t width = m_Description.Width;
	const uint32_t height = m_Description.Height;

	ASSERT(m_Description.Width != 0 && m_Description.Height != 0);

	return static_cast<uint32_t>(glm::floor(std::log2(glm::max(width, height)))) + 1;
}

const TextureDescription& Texture::GetDescription() const
{
	return m_Description;
}

Ref<Texture2D> Texture2D::Create(const std::string_view path)
{
	TextureDescription desc;
	desc.Format = Format::RGBA_8_SRGB;

	std::array<std::string_view, 1> paths = { path };

	return TryCreate<Texture2D>(paths, desc);
}

Texture2D::Texture2D(void** data, const TextureDescription& desc)
	: Texture(TextureType::TEXTURE, desc)
{
	CreateTexture(data);
	CreateSampler();
}

Texture2D::~Texture2D()
{
	m_Sampler.reset();
	m_Image.reset();
}

const Image2D& Texture2D::GetImage() const
{
	return *m_Image;
}

const Sampler& Texture2D::GetSampler() const
{
	return *m_Sampler;
}

void Texture2D::CreateTexture(void** data)
{
	ASSERT(data && data[0]);

	const auto& texDesc = Texture::GetDescription();
	const uint32_t width = texDesc.Width;
	const uint32_t height = texDesc.Height;

	ASSERT(width != 0 && height != 0);

	static constexpr uint32_t s_ImageCount = 1;

	VkDeviceSize imageSize = width * height * FormatBytesPerPixel(texDesc.Format);

	Scope<Buffer> stagingBuffer = Buffer::CreateStaging(imageSize);
	stagingBuffer->SetData(data[0]);

	ImageDescription desc;

	desc.Width = width;
	desc.Height = height;
	desc.MipLevels = texDesc.GenerateMipLevels ? GenerateMips() : 1;
	desc.ImageCount = s_ImageCount;
	desc.MSAAnumSamples = 1;
	desc.Format = texDesc.Format;
	desc.ImageUsage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
	desc.ImageCreateFlags = 0;
	desc.ImageAspectFlags = VK_IMAGE_ASPECT_COLOR_BIT;
	desc.ViewType = VK_IMAGE_VIEW_TYPE_2D;
	desc.Properties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;

	m_Image = Image2D::Create(desc);

	m_Image->TransitionImageLayout(VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
	m_Image->CopyFrom(*stagingBuffer);
}

void Texture2D::CreateSampler()
{
	SamplerDescription desc;

	desc.MipLevels = Texture::GetDescription().GenerateMipLevels ? GenerateMips() : 1;
	desc.MagFilter = Filter::NEAREST;
	desc.MinFilter = Filter::NEAREST;

	m_Sampler = Sampler::Create(desc);
}

Ref<TextureCube> TextureCube::Create(const std::array<std::string_view, 6>& paths)
{
	TextureDescription desc;
	desc.Format = Format::RGBA_8_SRGB;

	return TryCreate<TextureCube>(paths, desc);
}

TextureCube::TextureCube(void** data, const TextureDescription& desc)
	: Texture(TextureType::CUBE, desc)
{
	CreateCube(data);
	CreateSampler();
}

TextureCube::~TextureCube()
{
	m_Sampler.reset();
	m_Image.reset();
}

const Image2D& TextureCube::GetImage() const
{
	return *m_Image;
}

const Sampler& TextureCube::GetSampler() const
{
	return *m_Sampler;
}

void TextureCube::CreateCube(void** data)
{
	ASSERT(data);

	const auto& texDesc = Texture::GetDescription();
	const uint32_t width = texDesc.Width;
	const uint32_t height = texDesc.Height;

	ASSERT(width != 0 && height != 0);

	static constexpr uint32_t s_ImageCount = 6;


	VkDeviceSize imageSize = width * height * FormatBytesPerPixel(texDesc.Format);
	VkDeviceSize totalImageSize = imageSize * s_ImageCount;

	Scope<Buffer> stagingBuffer = Buffer::CreateStaging(totalImageSize);

	for (uint32_t i = 0; i < s_ImageCount; i++)
	{
		if (data[i])
			stagingBuffer->SetData(data[i], imageSize, imageSize * i);
	}

	ImageDescription desc;

	desc.Width = width;
	desc.Height = height;
	desc.MipLevels = texDesc.GenerateMipLevels ? GenerateMips() : 1;
	desc.ImageCount = s_ImageCount;
	desc.MSAAnumSamples = 1;
	desc.Format = texDesc.Format;
	desc.ImageUsage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
	desc.ImageCreateFlags = VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;
	desc.ImageAspectFlags = VK_IMAGE_ASPECT_COLOR_BIT;
	desc.ViewType = VK_IMAGE_VIEW_TYPE_CUBE;
	desc.Properties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;

	m_Image = Image2D::Create(desc);

	m_Image->TransitionImageLayout(VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
	m_Image->CopyFrom(*stagingBuffer);
}

void TextureCube::CreateSampler()
{
	SamplerDescription desc;

	desc.MipLevels = Texture::GetDescription().GenerateMipLevels ? GenerateMips() : 1;
	desc.MagFilter = Filter::NEAREST;
	desc.MinFilter = Filter::NEAREST;

	m_Sampler = Sampler::Create(desc);
}
