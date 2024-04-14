#pragma once

#include "Base.h"

#include "VK.h"

#include "Enums.h"

#include <glm/glm.hpp>

#include <span>

class Image2D;
class RenderPass;

struct FramebufferAttachment
{
	AttachmentType AttachType = AttachmentType::UNDEFINED;
	Format Format = Format::UNDEFINED;
	TextureType TexType = TextureType::UNDEFINED;
};

struct FramebufferDescription
{
	uint32_t Width = 0;
	uint32_t Height = 0;
	glm::vec4 ClearColor = { 0.0f, 0.0f, 0.0f, 1.0f };

	RenderPass* RenderPass = nullptr;
	std::span<const Image2D* const> Attachments;
};

class Framebuffer : public Handle<VkFramebuffer>
{
public:
	static Ref<Framebuffer> Create(const FramebufferDescription& desc);

	Framebuffer(const FramebufferDescription& desc);
	~Framebuffer();

	const FramebufferDescription& GetDescription() const;
private:
	void CreateFramebuffer();
private:
	FramebufferDescription m_Description;
};