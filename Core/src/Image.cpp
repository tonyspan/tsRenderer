#include "Image.h"

#include "Context.h"
#include "Device.h"
#include "CommandBuffer.h"
#include "GBuffer.h"

#include "Log.h"

#include <volk.h>
#include <vulkan/vulkan.h>

static bool HasStencil(VkFormat format)
{
	return format == VK_FORMAT_D32_SFLOAT_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT;
}

#pragma region Image

Ref<Image2D> Image2D::Create(const ImageDescription& desc)
{
	ASSERT(!desc.IsSwapchainImage);

	return CreateRef<Image2D>(desc);
}

Ref<Image2D> Image2D::Create(const ImageDescription& desc, VkImage image)
{
	ASSERT(desc.IsSwapchainImage);
	ASSERT(image);

	return CreateRef<Image2D>(desc, image);
}

Image2D::Image2D(const ImageDescription& desc)
	: m_Description(desc)
{
	CreateImage();
	CreateImageView();
}

Image2D::Image2D(const ImageDescription& desc, VkImage image)
	: m_Description(desc)
{
	Handle::GetHandle<VkImage>() = image;
	CreateImageView();
}

Image2D::~Image2D()
{
	const auto& vkDevice = Context::GetDevice().GetHandle();

	vkDestroyImageView(vkDevice, Handle::GetHandle<VkImageView>(), nullptr);

	if (!m_Description.IsSwapchainImage)
	{
		vkDestroyImage(vkDevice, Handle::GetHandle<VkImage>(), nullptr);
		vkFreeMemory(vkDevice, Handle::GetHandle<VkDeviceMemory>(), nullptr);
	}
}

void Image2D::TransitionImageLayout(VkImageLayout newLayout, VkImageLayout oldLayout)
{
	Ref<CommandBuffer> commandBuffer = CommandBuffer::Create(true);
	commandBuffer->BeginRecording(true);

	VkImageMemoryBarrier barrier;
	ZeroInitVkStruct(barrier, VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER);

	barrier.oldLayout = oldLayout;
	barrier.newLayout = newLayout;
	barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.image = Handle::GetHandle<VkImage>();

	if (newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
	{
		barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;

		const VkFormat vkFormat = Convert(m_Description.Format);
		if (HasStencil(vkFormat))
			barrier.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
	}
	else
	{
		barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	}

	barrier.subresourceRange.baseMipLevel = 0;
	barrier.subresourceRange.levelCount = m_Description.MipLevels;
	barrier.subresourceRange.baseArrayLayer = 0;
	barrier.subresourceRange.layerCount = m_Description.ImageCount;

	VkPipelineStageFlags sourceStage;
	VkPipelineStageFlags destinationStage;

	if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
	{
		barrier.srcAccessMask = 0;
		barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

		sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
		destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
	}
	else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
	{
		barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
		sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
		destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
	}
	else if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
	{
		barrier.srcAccessMask = 0;
		barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
		sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
		destinationStage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
	}
	else
	{
		ASSERT(false, "Unsupported layout transition");
	}

	vkCmdPipelineBarrier(commandBuffer->GetHandle(), sourceStage, destinationStage, 0, 0, nullptr, 0, nullptr, 1, &barrier);

	commandBuffer->EndRecording();
	commandBuffer->Submit();
}

void Image2D::CopyFrom(const GBuffer& buffer)
{
	Ref<CommandBuffer> commandBuffer = CommandBuffer::Create(true);
	commandBuffer->BeginRecording(true);

	VkBufferImageCopy region = {};

	region.bufferOffset = 0;
	region.bufferRowLength = 0;
	region.bufferImageHeight = 0;
	region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	region.imageSubresource.mipLevel = 0;
	region.imageSubresource.baseArrayLayer = 0;
	region.imageSubresource.layerCount = m_Description.ImageCount;
	region.imageOffset = { 0, 0, 0 };
	region.imageExtent = { m_Description.Width, m_Description.Height, 1 };

	vkCmdCopyBufferToImage(commandBuffer->GetHandle(), buffer.GetHandle<VkBuffer>(), Handle::GetHandle<VkImage>(), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

	commandBuffer->EndRecording();
	commandBuffer->Submit();

	GenerateMipMaps();
}

uint32_t Image2D::GetWidth() const
{
	return m_Description.Width;
}

uint32_t Image2D::GetHeight() const
{
	return m_Description.Height;
}

Format Image2D::GetFormat() const
{
	return m_Description.Format;
}

void Image2D::CreateImage()
{
	const auto& physicalDevice = Context::GetDevice().GetPhysicalDevice();
	const auto& device = Context::GetDevice().GetHandle();

	ASSERT(0 < m_Description.ImageCount);
	ASSERT(0 < m_Description.Width && 0 < m_Description.Height);

	VkImageCreateInfo imageInfo;
	ZeroInitVkStruct(imageInfo, VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO);

	imageInfo.imageType = VK_IMAGE_TYPE_2D;
	imageInfo.extent.width = m_Description.Width;
	imageInfo.extent.height = m_Description.Height;
	imageInfo.extent.depth = 1;
	imageInfo.mipLevels = m_Description.MipLevels;
	imageInfo.arrayLayers = m_Description.ImageCount;
	imageInfo.format = Convert(m_Description.Format);
	imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
	imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	imageInfo.usage = m_Description.ImageUsage;
	imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	imageInfo.samples = Convert(m_Description.MSAAnumSamples);
	imageInfo.flags = m_Description.ImageCreateFlags;

	auto& imageHandle = Handle::GetHandle<VkImage>();

	VkResult result = vkCreateImage(device, &imageInfo, nullptr, &imageHandle);
	VK_CHECK_RESULT(result);
	ASSERT(imageHandle, "Image creation failed");

	VkMemoryRequirements memRequirements;
	vkGetImageMemoryRequirements(device, imageHandle, &memRequirements);

	VkMemoryAllocateInfo allocInfo;
	ZeroInitVkStruct(allocInfo, VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO);

	allocInfo.allocationSize = memRequirements.size;
	allocInfo.memoryTypeIndex = physicalDevice.GetMemoryType(memRequirements.memoryTypeBits, m_Description.Properties);

	auto& memoryHandle = Handle::GetHandle<VkDeviceMemory>();

	result = vkAllocateMemory(device, &allocInfo, nullptr, &memoryHandle);
	VK_CHECK_RESULT(result);
	ASSERT(memoryHandle, "Failed to allocate image memory");

	result = vkBindImageMemory(device, imageHandle, memoryHandle, 0);
	VK_CHECK_RESULT(result);
}

void Image2D::CreateImageView()
{
	auto& imageHandle = Handle::GetHandle<VkImage>();

	ASSERT(imageHandle);

	VkImageViewCreateInfo viewInfo;
	ZeroInitVkStruct(viewInfo, VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO);

	viewInfo.image = imageHandle;
	viewInfo.viewType = m_Description.ViewType;
	viewInfo.format = Convert(m_Description.Format);
	viewInfo.subresourceRange.aspectMask = m_Description.ImageAspectFlags;
	viewInfo.subresourceRange.baseMipLevel = 0;
	viewInfo.subresourceRange.levelCount = m_Description.MipLevels;
	viewInfo.subresourceRange.baseArrayLayer = 0;
	viewInfo.subresourceRange.layerCount = m_Description.ImageCount;

	auto& imageViewHandle = Handle::GetHandle<VkImageView>();

	VkResult result = vkCreateImageView(Context::GetDevice().GetHandle(), &viewInfo, nullptr, &imageViewHandle);
	VK_CHECK_RESULT(result);
	ASSERT(imageViewHandle, "ImageView creation failed");
}

void Image2D::GenerateMipMaps()
{
	const auto& physicalDevice = Context::GetDevice().GetPhysicalDevice().GetHandle();

	VkFormatProperties formatProperties;
	vkGetPhysicalDeviceFormatProperties(physicalDevice, Convert(m_Description.Format), &formatProperties);

	ASSERT(formatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT, "Image format doesn't support linear blitting");

	Ref<CommandBuffer> commandBuffer = CommandBuffer::Create(true);
	commandBuffer->BeginRecording(true);

	VkImageMemoryBarrier barrier;
	ZeroInitVkStruct(barrier, VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER);

	auto& imageHandle = Handle::GetHandle<VkImage>();

	barrier.image = imageHandle;
	barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	barrier.subresourceRange.baseArrayLayer = 0;
	barrier.subresourceRange.layerCount = m_Description.ImageCount;
	barrier.subresourceRange.levelCount = 1;

	int32_t mipWidth = m_Description.Width;
	int32_t mipHeight = m_Description.Height;

	const VkImageLayout layoutTransferDstOptimal = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
	const VkImageLayout layoutTransferSrcOptimal = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
	for (uint32_t i = 1; i < m_Description.MipLevels; i++)
	{
		barrier.subresourceRange.baseMipLevel = i - 1;
		barrier.oldLayout = layoutTransferDstOptimal;
		barrier.newLayout = layoutTransferSrcOptimal;
		barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;

		vkCmdPipelineBarrier(commandBuffer->GetHandle(), VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 0, nullptr, 1, &barrier);

		VkImageBlit blit = {};

		blit.srcOffsets[0] = { 0, 0, 0 };
		blit.srcOffsets[1] = { mipWidth, mipHeight, 1 };
		blit.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		blit.srcSubresource.mipLevel = i - 1;
		blit.srcSubresource.baseArrayLayer = 0;
		blit.srcSubresource.layerCount = m_Description.ImageCount;
		blit.dstOffsets[0] = { 0, 0, 0 };

		blit.dstOffsets[1] = { mipWidth > 1 ? mipWidth / 2 : 1,
							   mipHeight > 1 ? mipHeight / 2 : 1, 1 };

		blit.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		blit.dstSubresource.mipLevel = i;
		blit.dstSubresource.baseArrayLayer = 0;
		blit.dstSubresource.layerCount = m_Description.ImageCount;

		vkCmdBlitImage(commandBuffer->GetHandle(), imageHandle, layoutTransferSrcOptimal, imageHandle, layoutTransferDstOptimal, 1, &blit, VK_FILTER_LINEAR);

		barrier.oldLayout = layoutTransferSrcOptimal;
		barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
		barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

		vkCmdPipelineBarrier(commandBuffer->GetHandle(), VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0, nullptr, 0, nullptr, 1, &barrier);

		if (mipWidth > 1)
			mipWidth /= 2;

		if (mipHeight > 1)
			mipHeight /= 2;
	}

	barrier.subresourceRange.baseMipLevel = m_Description.MipLevels - 1;
	barrier.oldLayout = layoutTransferDstOptimal;
	barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
	barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
	vkCmdPipelineBarrier(commandBuffer->GetHandle(), VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0, nullptr, 0, nullptr, 1, &barrier);

	commandBuffer->EndRecording();
	commandBuffer->Submit();
}
