#include "Framebuffer.h"

#include "Log.h"

#include "Device.h"
#include "Image.h"
#include "RenderPass.h"
#include "Swapchain.h"

#include <vulkan/vulkan.h>

#include <array>

FramebufferDescription::FramebufferDescription()
	: Width(0), Height(0), RenderPass(nullptr)
	, Format(VK_FORMAT_UNDEFINED), ClearColor({ 0.0f, 0.0f, 0.0f, 1.0f })
{
}

Ref<Framebuffer> Framebuffer::Create(Device& device, const FramebufferDescription& desc)
{
	return CreateRef<Framebuffer>(device, desc);
}

Framebuffer::Framebuffer(Device& device, const FramebufferDescription& desc)
	: m_Device(device), m_Description(desc)
{
	CreateFramebuffer();
}

Framebuffer::~Framebuffer()
{
	vkDestroyFramebuffer(m_Device.GetHandle(), Handle::GetHandle(), nullptr);
}

const FramebufferDescription& Framebuffer::GetDescription() const
{
	return m_Description;
}

void Framebuffer::CreateFramebuffer()
{
	auto renderPass = m_Description.RenderPass;

	ASSERT(renderPass, "RenderPass must be valid");
	ASSERT(m_Description.Width != 0 && m_Description.Height != 0, STR(m_Description.Width, m_Description.Height) " aren't set properly");

	const auto& descAttachments = m_Description.Attachments;

	std::vector<VkImageView> attachments(descAttachments.size());
	for (size_t i = 0; i < descAttachments.size(); i++)
		attachments[i] = descAttachments[i]->GetImageViewHandle();

	VkFramebufferCreateInfo framebufferInfo;
	ZeroInitVkStruct(framebufferInfo, VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO);

	framebufferInfo.renderPass = renderPass->GetHandle();
	framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
	framebufferInfo.pAttachments = attachments.data();
	framebufferInfo.width = m_Description.Width;
	framebufferInfo.height = m_Description.Height;
	framebufferInfo.layers = 1;

	VkResult result = vkCreateFramebuffer(m_Device.GetHandle(), &framebufferInfo, nullptr, &Handle::GetHandle());
	VK_CHECK_RESULT(result);
	ASSERT(Handle::GetHandle(), "Framebuffer creation failed");
}