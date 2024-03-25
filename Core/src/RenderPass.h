#pragma once

#include "Base.h"

#include "VK.h"

#include <span>
#include <optional>

class CommandBuffer;
class Framebuffer;
class Image2D;

struct RenderPassDescription
{
	std::optional<uint8_t> MSAAnumSamples = {};
	std::span<const Image2D* const> Attachments;
};

class RenderPass : public Handle<VkRenderPass>
{
public:
	static Ref<RenderPass> Create(const RenderPassDescription& desc);

	RenderPass(const RenderPassDescription& desc);
	~RenderPass();

	void Begin(const CommandBuffer& commandBuffer, const Framebuffer& framebuffer);
	void End(const CommandBuffer& commandBuffer);
private:
	void CreateRenderPass(const RenderPassDescription& desc);
};