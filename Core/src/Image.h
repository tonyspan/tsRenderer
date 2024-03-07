#pragma once

#include "Base.h"

#include "VK.h"

#include "Enums.h"

#pragma region Image

struct ImageDescription
{
	uint32_t Width;
	uint32_t Height;
	uint32_t MipLevels;
	uint32_t ImageCount;
	uint32_t MSAAnumSamples;
	Format Format;
	VkImageUsageFlags ImageUsage;
	VkImageCreateFlags ImageCreateFlags;
	VkImageAspectFlags ImageAspectFlags;

	VkImageViewType ViewType;

	VkMemoryPropertyFlags Properties;

	bool IsSwapchainImage = false;

	ImageDescription();
};

struct ImageHandle
{
	VkImage Image;
	VkImageView ImageView;
	VkDeviceMemory Memory;

	ImageHandle();
	~ImageHandle() = default;
};

class Buffer;

class Image2D
{
public:
	static Ref<Image2D> Create(const ImageDescription& desc);
	// For Swapchain images
	static Ref<Image2D> Create(const ImageDescription& desc, VkImage image);

	Image2D(const ImageDescription& desc);
	Image2D(const ImageDescription& desc, VkImage image);
	~Image2D();

	VkImage GetImageHandle() const;
	VkImageView GetImageViewHandle() const;

	void TransitionImageLayout(VkImageLayout newLayout, VkImageLayout oldLayout = (VkImageLayout)0);
	void CopyFrom(const Buffer& buffer);

	uint32_t GetWidth() const;
	uint32_t GetHeight() const;
	Format GetFormat() const;
	uint32_t GetMSAAsamples() const;
private:
	void CreateImage();
	void CreateImageView();

	void GenerateMipMaps();
private:
	ImageHandle m_Image;
	ImageDescription m_Description;
};