#pragma once

#include "Base.h"

#include "VK.h"

#include <span>
#include <optional>

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

	const RenderPassDescription& GetDescription() const;
private:
	void CreateRenderPass();
private:
	RenderPassDescription m_Description;
};