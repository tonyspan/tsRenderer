#include "Swapchain.h"

#include "Device.h"
#include "Surface.h"
#include "Image.h"
#include "RenderPass.h"
#include "CommandBuffer.h"
#include "Synchronization.h"
#include "Framebuffer.h"

#include "Log.h"
#include "Profiler.h"

#include <vulkan/vulkan.h>

#include <glm/glm.hpp>

#include <array>

Swapchain::Swapchain(Device& device, Surface& surface, const SwapchainDescription& desc)
	: m_Device(device), m_Surface(surface), m_Description(desc)
{
	CreateAll();
}

Swapchain::~Swapchain()
{
	Destroy();
}

void Swapchain::BeginFrame()
{
	PROFILE_FUNCTION();

	constexpr uint64_t timeout = std::numeric_limits<uint64_t>::max();

	auto& currentFence = GetCurrentFence();
	currentFence.Wait(timeout);

	auto& presentFinished = GetCurrentSemaphores().PresentFinished->GetHandle();

	VkResult result = vkAcquireNextImageKHR(m_Device.GetHandle(), Handle::GetHandle(), timeout, presentFinished, VK_NULL_HANDLE, &m_ImageIndex);
	VK_CHECK_RESULT(result);

	currentFence.Reset();

	if (result == VK_ERROR_OUT_OF_DATE_KHR)
	{
		OnResize(m_Description.Width, m_Description.Height);
	}
	ASSERT(result == VK_SUCCESS || result == VK_SUBOPTIMAL_KHR, "Failed to acquire Swapchain image");

	auto& cmdBuffer = GetCurrentCommandBuffer();
	cmdBuffer.BeginRecording();
	m_RenderPass->Begin(cmdBuffer, GetCurrentFramebuffer());
}

void Swapchain::EndFrame()
{
	auto& cmdBuffer = GetCurrentCommandBuffer();
	m_RenderPass->End(cmdBuffer);
	cmdBuffer.EndRecording();

	std::array waitSemaphores = { GetCurrentSemaphores().PresentFinished->GetHandle() };
	std::array signalSemaphores = { GetCurrentSemaphores().RenderFinished->GetHandle() };
	std::array commandBuffers = { GetCurrentCommandBuffer().GetHandle() };

	Submit(waitSemaphores, signalSemaphores, commandBuffers);

	Present(signalSemaphores);

	m_CurrentFrame = ++m_CurrentFrame % GetImageCount();
}

void Swapchain::OnResize(uint32_t width, uint32_t height)
{
	PROFILE_FUNCTION();

	if (m_Description.Width == width && m_Description.Height == height)
		return;

	m_Description.Width = width;
	m_Description.Height = height;

	//m_Device.WaitIdle();

	Destroy();

	CreateAll();

	//m_Device.WaitIdle();
}

const SwapchainDescription& Swapchain::GetDescription() const
{
	return m_Description;
}

const uint32_t Swapchain::GetImageCount() const
{
	// TODO: Remove
	ASSERT(m_FrameData.size() == m_Description.FramesInFlight);

	return static_cast<uint32_t>(m_FrameData.size());
}

const uint32_t Swapchain::GetCurrentImage() const
{
	return m_ImageIndex;
}

const Image2D* Swapchain::GetImage(uint32_t index) const
{
	return GetFrameData(index).Image.get();
}

const uint32_t Swapchain::GetCurrentFrame() const
{
	return m_CurrentFrame;
}

Ref<RenderPass> Swapchain::GetRenderPass() const
{
	return m_RenderPass;
}

CommandBuffer& Swapchain::GetCurrentCommandBuffer() const
{
	return *GetFrameData(m_ImageIndex).CommandBuffer;
}

CommandBuffer& Swapchain::GetCurrentCommandBuffer()
{
	return *GetFrameData(m_ImageIndex).CommandBuffer;
}

const Framebuffer& Swapchain::GetCurrentFramebuffer() const
{
	return *GetFrameData(m_ImageIndex).Framebuffer;
}

void Swapchain::CreateSwapchain()
{
	ASSERT(m_Description.FramesInFlight > 0, STR(m_Description.FramesInFlight) " <= 0");
	ASSERT(m_Description.Width != 0 && m_Description.Height != 0, STR(m_Description.Width, m_Description.Height) " == 0");

	const PhysicalDevice& physicalDevice = m_Device.GetPhysicalDevice();

	const auto& surfaceCapabilities = m_Surface.GetCapabilities(physicalDevice);
	VkSurfaceFormatKHR surfaceFormat = m_Surface.GetFormat(physicalDevice);
	VkPresentModeKHR presentMode = m_Surface.GetPresentMode(physicalDevice, m_Description.VSync);

	m_ImageFormat = surfaceFormat.format;

	VkExtent2D extent = { m_Description.Width, m_Description.Height };

	uint32_t& framesInFlight = m_Description.FramesInFlight;
	framesInFlight = glm::clamp(m_Description.FramesInFlight, surfaceCapabilities.minImageCount, surfaceCapabilities.maxImageCount);

	m_FrameData.resize(framesInFlight);

	VkSwapchainCreateInfoKHR createInfo;
	ZeroInitVkStruct(createInfo, VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR);

	createInfo.surface = m_Surface.GetHandle();
	createInfo.minImageCount = framesInFlight;
	createInfo.imageFormat = m_ImageFormat;
	createInfo.imageColorSpace = surfaceFormat.colorSpace;
	createInfo.imageExtent = extent;
	createInfo.imageArrayLayers = 1;
	createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

	const auto& queueFamilyIndices = m_Device.GetPhysicalDevice().GetQueueFamilyIndices();

	std::array<uint32_t, 2> queueFamilyIndicesValues;
	if (queueFamilyIndices.IsComplete() && !queueFamilyIndices.IsSame())
	{
		queueFamilyIndicesValues = { queueFamilyIndices.GraphicsIndex.value(), queueFamilyIndices.PresentIndex.value() };

		createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;

		createInfo.queueFamilyIndexCount = static_cast<uint32_t>(queueFamilyIndicesValues.size());
		createInfo.pQueueFamilyIndices = queueFamilyIndicesValues.data();
	}
	else
	{
		createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
	}

	createInfo.preTransform = surfaceCapabilities.currentTransform;
	createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	createInfo.presentMode = presentMode;
	createInfo.clipped = VK_TRUE;
	createInfo.oldSwapchain = VK_NULL_HANDLE;

	VkResult result = vkCreateSwapchainKHR(m_Device.GetHandle(), &createInfo, nullptr, &Handle::GetHandle());
	VK_CHECK_RESULT(result);
	ASSERT(Handle::GetHandle(), "Swapchain creation failed");
}

void Swapchain::CreateImagesAndViews()
{
	uint32_t imageCount = GetImageCount();

	VkImage* swapchainImages = new VkImage[imageCount];

	VkResult result = vkGetSwapchainImagesKHR(m_Device.GetHandle(), Handle::GetHandle(), &imageCount, swapchainImages);
	VK_CHECK_RESULT(result);

	Format format = Format::BGRA_8_SRGB;
	ASSERT(Convert(format) == m_ImageFormat);

	ImageDescription desc;

	desc.Format = format;
	desc.ImageAspectFlags = VK_IMAGE_ASPECT_COLOR_BIT;
	desc.ViewType = VK_IMAGE_VIEW_TYPE_2D;
	desc.MipLevels = 1;
	desc.ImageCount = 1;
	desc.IsSwapchainImage = true;

	for (uint32_t i = 0; auto & frameData : m_FrameData)
	{
		frameData.Image = Image2D::Create(desc, swapchainImages[i]);

		i++;
	}

	delete[] swapchainImages;
	swapchainImages = nullptr;
}

// TODO: Find a better way
// This setting affects Pipeline, RenderPass, ImGui
static constexpr uint8_t s_MSAA = 1;

void Swapchain::CreateColorResources()
{
	Format format = Format::BGRA_8_SRGB;
	ASSERT(Convert(format) == m_ImageFormat);

	ImageDescription desc;

	desc.Width = m_Description.Width;
	desc.Height = m_Description.Height;
	desc.MipLevels = 1;
	desc.ImageCount = 1;
	desc.MSAAnumSamples = s_MSAA;
	desc.Format = format;
	desc.ImageUsage = VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
	desc.ImageAspectFlags = VK_IMAGE_ASPECT_COLOR_BIT;
	desc.ImageCreateFlags = 0;
	desc.ViewType = VK_IMAGE_VIEW_TYPE_2D;
	desc.Properties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;

	m_ColorImage = Image2D::Create(desc);
}

void Swapchain::CreateDepthResources()
{
	Format format = Format::D32_SFLOAT;
	ASSERT(m_Device.GetPhysicalDevice().GetDepthFormat() == Convert(format));

	ImageDescription desc;

	desc.Width = m_Description.Width;
	desc.Height = m_Description.Height;
	desc.MipLevels = 1;
	desc.ImageCount = 1;
	desc.MSAAnumSamples = s_MSAA;
	desc.Format = format;
	desc.ImageUsage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
	desc.ImageAspectFlags = VK_IMAGE_ASPECT_DEPTH_BIT;
	desc.ImageCreateFlags = 0;
	desc.ViewType = VK_IMAGE_VIEW_TYPE_2D;
	desc.Properties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;

	m_DepthImage = Image2D::Create(desc);
}

void Swapchain::CreateRenderPass()
{
	RenderPassDescription desc;

	desc.MSAAnumSamples = s_MSAA;

	std::array<const Image2D*, 3> attachments{};
	if constexpr (s_MSAA > 1)
	{
		attachments = { m_ColorImage.get(), m_DepthImage.get(), GetImage(0) };
	}
	else
	{
		attachments = { GetImage(0), m_DepthImage.get() };
	}

	desc.Attachments = attachments;

	m_RenderPass = RenderPass::Create(desc);
}

void Swapchain::CreateFramebuffers()
{
	FramebufferDescription desc;

	desc.Width = m_Description.Width;
	desc.Height = m_Description.Height;
	desc.ClearColor = { 0.0f, 0.0f, 0.0f, 1.0f };
	desc.RenderPass = m_RenderPass.get();
	desc.MSAAnumSamples = s_MSAA;

	for (uint32_t i = 0; i < m_FrameData.size(); i++)
	{
		std::array<const Image2D*, 3> attachments{};
		if constexpr (s_MSAA > 1)
		{
			attachments = { m_ColorImage.get(), m_DepthImage.get(), GetImage(i) };
		}
		else
		{
			attachments = { GetImage(i), m_DepthImage.get() };
		}

		desc.Attachments = attachments;

		m_FrameData[i].Framebuffer = Framebuffer::Create(desc);
	}
}

void Swapchain::CreateCommandBuffers()
{
	for (auto& frameData : m_FrameData)
	{
		auto& cmdBuffer = frameData.CommandBuffer;
		cmdBuffer = CommandBuffer::Create(true);
	}
}

void Swapchain::CreateSyncObjects()
{
	for (auto& frameData : m_FrameData)
	{
		auto& semaphores = frameData.Semaphoress;
		semaphores.PresentFinished = Semaphore::Create(m_Device);
		semaphores.RenderFinished = Semaphore::Create(m_Device);

		FenceDescription desc;
		desc.CreateFlags = VK_FENCE_CREATE_SIGNALED_BIT;

		frameData.Fence = Fence::Create(m_Device, desc);
	}
}

void Swapchain::CreateAll()
{
	CreateSwapchain();
	CreateImagesAndViews();
	CreateColorResources();
	CreateDepthResources();
	CreateRenderPass();
	CreateFramebuffers();
	CreateCommandBuffers();
	CreateSyncObjects();
}

void Swapchain::Destroy()
{
	m_RenderPass.reset();

	m_ColorImage.reset();

	m_DepthImage.reset();

	const auto& device = m_Device.GetHandle();

	for (auto& frameData : m_FrameData)
	{
		frameData.Framebuffer.reset();

		frameData.CommandBuffer.reset();

		auto& semaphores = frameData.Semaphoress;
		semaphores.PresentFinished.reset();
		semaphores.RenderFinished.reset();

		frameData.Fence.reset();

		frameData.Image.reset();
	}

	m_FrameData.clear();

	vkDestroySwapchainKHR(device, Handle::GetHandle(), nullptr);
}

void Swapchain::Submit(const std::span<const VkSemaphore> waitSemaphore, const std::span<const VkSemaphore> signalSemaphore,
	const std::span<const VkCommandBuffer> commandBuffer)
{
	PROFILE_FUNCTION();

	ASSERT(waitSemaphore.size() == 1 || signalSemaphore.size() == 1 || commandBuffer.size() == 1);

	VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };

	VkSubmitInfo submitInfo;
	ZeroInitVkStruct(submitInfo, VK_STRUCTURE_TYPE_SUBMIT_INFO);

	submitInfo.waitSemaphoreCount = static_cast<uint32_t>(waitSemaphore.size());
	submitInfo.pWaitSemaphores = waitSemaphore.data();
	submitInfo.pWaitDstStageMask = waitStages;
	submitInfo.commandBufferCount = static_cast<uint32_t>(commandBuffer.size());
	submitInfo.pCommandBuffers = commandBuffer.data();
	submitInfo.signalSemaphoreCount = static_cast<uint32_t>(signalSemaphore.size());
	submitInfo.pSignalSemaphores = signalSemaphore.data();

	VkResult result = vkQueueSubmit(m_Device.GetGraphicsQueue(), 1, &submitInfo, GetCurrentFence().GetHandle());
	VK_CHECK_RESULT(result);
}

void Swapchain::Present(const std::span<const VkSemaphore> signalSemaphore)
{
	PROFILE_FUNCTION();

	std::array swapchains = { Handle::GetHandle() };

	VkPresentInfoKHR presentInfo;
	ZeroInitVkStruct(presentInfo, VK_STRUCTURE_TYPE_PRESENT_INFO_KHR);

	presentInfo.waitSemaphoreCount = static_cast<uint32_t>(signalSemaphore.size());
	presentInfo.pWaitSemaphores = signalSemaphore.data();
	presentInfo.swapchainCount = static_cast<uint32_t>(swapchains.size());
	presentInfo.pSwapchains = swapchains.data();
	presentInfo.pImageIndices = &m_ImageIndex;

	VkResult result = vkQueuePresentKHR(m_Device.GetPresentQueue(), &presentInfo);
	VK_CHECK_RESULT(result);

	if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR)
	{
		OnResize(m_Description.Width, m_Description.Height);
	}
	else if (result != VK_SUCCESS)
	{
		ASSERT(false, "Failed to present Swapchain");
	}
}

const Swapchain::FrameData& Swapchain::GetFrameData(uint32_t index) const
{
	ASSERT(index >= 0 && index <= GetImageCount());

	return m_FrameData.at(index);
}

Swapchain::FrameData& Swapchain::GetFrameData(uint32_t index)
{
	ASSERT(index >= 0 && index <= GetImageCount());

	return m_FrameData.at(index);
}

Framebuffer& Swapchain::GetCurrentFramebuffer()
{
	return *GetFrameData(m_ImageIndex).Framebuffer;
}

Fence& Swapchain::GetCurrentFence()
{
	return *GetFrameData(m_CurrentFrame).Fence;
}

Swapchain::Semaphores& Swapchain::GetCurrentSemaphores()
{
	return GetFrameData(m_CurrentFrame).Semaphoress;
}
