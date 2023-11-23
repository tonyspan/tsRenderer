#pragma once

#include "Base.h"

#include "VK.h"

class Device;
class Swapchain;
class CommandBuffer;
class Framebuffer;

class RenderPassDescription
{
};

class RenderPass : public Handle<VkRenderPass>
{
public:
	static Ref<RenderPass> Create(const Device& device, const Swapchain& swapchain);

	RenderPass(const Device& device, const Swapchain& swapchain);
	~RenderPass();

	void Begin(const CommandBuffer& commandBuffer, const Framebuffer& framebuffer);
	void End(const CommandBuffer& commandBuffer);
private:
	void CreateRenderPass(const Swapchain& swapchain);
private:
	const Device& m_Device;
};