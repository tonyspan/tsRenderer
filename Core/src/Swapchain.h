#pragma once

#include "Base.h"

#include "VK.h"

#include <vector>
#include <span>

class Device;
class Surface;

class Image2D;

class RenderPass;
class CommandBuffer;
class Framebuffer;

class Semaphore;
class Fence;

struct SwapchainDescription
{
	uint32_t FramesInFlight = 3;

	uint32_t Width = 0;
	uint32_t Height = 0;

	bool VSync = false;
};

class Swapchain : public Handle<VkSwapchainKHR>
{
	struct Semaphores
	{
		Ref<Semaphore> RenderFinished;
		Ref<Semaphore> PresentFinished;
	};

	struct FrameData
	{
		Ref<CommandBuffer> CommandBuffer;
		Ref<Framebuffer> Framebuffer;
		Ref<Fence> Fence;
		Semaphores Semaphoress;
		Ref<Image2D> Image;
	};
public:
	Swapchain(Device& device, Surface& surface, const SwapchainDescription& desc);
	~Swapchain();

	void BeginFrame();
	void EndFrame();

	void OnResize(uint32_t width, uint32_t height);

	const SwapchainDescription& GetDescription() const;

	const uint32_t GetImageCount() const;
	const uint32_t GetCurrentImage() const;
	const Image2D* GetImage(uint32_t index) const;
	const uint32_t GetCurrentFrame() const;
	Ref<RenderPass> GetRenderPass() const;

	const CommandBuffer& GetCurrentCommandBuffer() const;
	CommandBuffer& GetCurrentCommandBuffer();

	const Framebuffer& GetCurrentFramebuffer() const;
private:
	void CreateSwapchain();
	void CreateImagesAndViews();
	void CreateColorResources();
	void CreateDepthResources();
	void CreateRenderPass();
	void CreateFramebuffers();
	void CreateCommandBuffers();
	void CreateSyncObjects();

	void CreateAll();
	void Destroy();

	void Submit(const std::span<const VkSemaphore> waitSemaphore, const std::span<const VkSemaphore> signalSemaphore, const std::span<const VkCommandBuffer> commandBuffer);
	void Present(const std::span<const VkSemaphore> signalSemaphore);

	const FrameData& GetFrameData(uint32_t index) const;
	FrameData& GetFrameData(uint32_t index);

	Fence& GetCurrentFence();
	Semaphores& GetCurrentSemaphores();
private:
	Device& m_Device;
	Surface& m_Surface;

	SwapchainDescription m_Description;

	VkFormat m_ImageFormat;

	Ref<Image2D> m_ColorImage;

	Ref<Image2D> m_DepthImage;

	std::vector<FrameData> m_FrameData;

	Ref<RenderPass> m_RenderPass;

	uint32_t m_CurrentFrame = 0;
	uint32_t m_ImageIndex = 0;
};