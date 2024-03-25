#pragma once

#include "Base.h"

#include "VK.h"

#include "Enums.h"

#pragma region Image

struct ImageDescription
{
	uint32_t Width = 0;
	uint32_t Height = 0;
	uint32_t MipLevels = 0;
	uint32_t ImageCount = 0;
	uint8_t MSAAnumSamples = 0;
	Format Format = Format::UNDEFINED;
	VkImageUsageFlags ImageUsage = (VkImageUsageFlags)VK_MAX_VALUE_ENUM;
	VkImageCreateFlags ImageCreateFlags = (VkImageUsageFlags)VK_MAX_VALUE_ENUM;
	VkImageAspectFlags ImageAspectFlags = (VkImageAspectFlags)VK_MAX_VALUE_ENUM;

	VkImageViewType ViewType = (VkImageViewType)VK_MAX_VALUE_ENUM;

	VkMemoryPropertyFlags Properties = (VkMemoryPropertyFlags)VK_MAX_VALUE_ENUM;

	bool IsSwapchainImage = false;
};

class Buffer;

class Image2D : public Handle<VkImage, VkImageView, VkDeviceMemory>
{
public:
	static Ref<Image2D> Create(const ImageDescription& desc);
	// For Swapchain images
	static Ref<Image2D> Create(const ImageDescription& desc, VkImage image);

	Image2D(const ImageDescription& desc);
	Image2D(const ImageDescription& desc, VkImage image);
	~Image2D();

	void TransitionImageLayout(VkImageLayout newLayout, VkImageLayout oldLayout = (VkImageLayout)0);
	void CopyFrom(const Buffer& buffer);

	uint32_t GetWidth() const;
	uint32_t GetHeight() const;
	Format GetFormat() const;
private:
	void CreateImage();
	void CreateImageView();

	void GenerateMipMaps();
private:
	ImageDescription m_Description;
};