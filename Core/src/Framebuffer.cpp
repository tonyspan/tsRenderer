#include "Framebuffer.h"

#include "Log.h"

#include "Context.h"
#include "Device.h"
#include "Image.h"
#include "RenderPass.h"
#include "Swapchain.h"

#include <volk.h>
#include <vulkan/vulkan.h>

#include <array>

Ref<Framebuffer> Framebuffer::Create(const FramebufferDescription& desc)
{
	return CreateRef<Framebuffer>(desc);
}

Framebuffer::Framebuffer(const FramebufferDescription& desc)
	: m_Description(desc)
{
	CreateFramebuffer();
}

Framebuffer::~Framebuffer()
{
	vkDestroyFramebuffer(Context::GetDevice().GetHandle(), Handle::GetHandle(), nullptr);
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

	std::vector<VkImageView> attachments{};
	for (const auto attachment : descAttachments)
	{
		if (attachment)
			attachments.push_back(attachment->GetHandle<VkImageView>());
	}

	VkFramebufferCreateInfo framebufferInfo;
	ZeroInitVkStruct(framebufferInfo, VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO);

	framebufferInfo.renderPass = renderPass->GetHandle();
	framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
	framebufferInfo.pAttachments = attachments.data();
	framebufferInfo.width = m_Description.Width;
	framebufferInfo.height = m_Description.Height;
	framebufferInfo.layers = 1;

	VkResult result = vkCreateFramebuffer(Context::GetDevice().GetHandle(), &framebufferInfo, nullptr, &Handle::GetHandle());
	VK_CHECK_RESULT(result);
	ASSERT(Handle::GetHandle(), "Framebuffer creation failed");
}