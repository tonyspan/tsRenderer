#pragma once

#include "Base.h"

#include "VK.h"

#include <glm/glm.hpp>

#include <span>

class Device;

class Image2D;
class RenderPass;

struct FramebufferDescription
{
	uint32_t Width;
	uint32_t Height;
	glm::vec4 ClearColor;

	RenderPass* RenderPass;
	std::span<Image2D*> Attachments;

	VkFormat Format;

	FramebufferDescription();
};

class Framebuffer : public Handle<VkFramebuffer>
{
public:
	static Ref<Framebuffer> Create(Device& device, const FramebufferDescription& desc);

	Framebuffer(Device& device, const FramebufferDescription& desc);
	~Framebuffer();

	const FramebufferDescription& GetDescription() const;
private:
	void CreateFramebuffer();
private:
	Device& m_Device;

	FramebufferDescription m_Description;
};