#include "RenderPass.h"

#include "Context.h"
#include "Device.h"
#include "Swapchain.h"
#include "CommandBuffer.h"
#include "Framebuffer.h"
#include "Image.h"

#include "Log.h"

#include <vulkan/vulkan.h>

#include <array>

Ref<RenderPass> RenderPass::Create(const RenderPassDescription& desc)
{
	return CreateRef<RenderPass>(desc);
}

RenderPass::RenderPass(const RenderPassDescription& desc)
{
	CreateRenderPass(desc);
}

RenderPass::~RenderPass()
{
	vkDestroyRenderPass(Context::GetDevice().GetHandle(), Handle::GetHandle(), nullptr);
}

void RenderPass::Begin(const CommandBuffer& commandBuffer, const Framebuffer& framebuffer)
{
	VkRenderPassBeginInfo renderPassInfo;
	ZeroInitVkStruct(renderPassInfo, VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO);

	const auto& framebufferDesc = framebuffer.GetDescription();
	const auto& clearColor = framebufferDesc.ClearColor;

	renderPassInfo.renderPass = Handle::GetHandle();
	renderPassInfo.framebuffer = framebuffer.GetHandle();
	renderPassInfo.renderArea.offset = { 0, 0 };
	renderPassInfo.renderArea.extent = { .width = framebufferDesc.Width, .height = framebufferDesc.Height };

	std::array<VkClearValue, 2> clearValues{};
	clearValues[0].color = { { clearColor.r, clearColor.g, clearColor.b, clearColor.a } };
	clearValues[1].depthStencil = { 1.0f, 0 };

	renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
	renderPassInfo.pClearValues = clearValues.data();

	vkCmdBeginRenderPass(commandBuffer.GetHandle(), &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
}

void RenderPass::End(const CommandBuffer& commandBuffer)
{
	vkCmdEndRenderPass(commandBuffer.GetHandle());
}

void RenderPass::CreateRenderPass(const RenderPassDescription& desc)
{
	const auto& physicalDevice = Context::GetDevice().GetPhysicalDevice();
	const auto& descAttachments = desc.Attachments;

	std::array<VkAttachmentDescription, 3> attachments{};
	std::array<VkAttachmentReference, attachments.size()> attachmentRefs{};

	ASSERT(descAttachments.size() == attachments.size());

	VkFormat vkFormat = VK_FORMAT_UNDEFINED;

	// Color
	const auto colorImage = descAttachments[0];
	vkFormat = Convert(colorImage->GetFormat());
	attachments[0].format = vkFormat;
	attachments[0].samples = Convert(colorImage->GetMSAAsamples());
	attachments[0].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	attachments[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	attachments[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	attachments[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	attachments[0].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	attachments[0].finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	attachmentRefs[0] = { .attachment = 0, .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL };

	// Depth
	const auto depthImage = descAttachments[1];
	vkFormat = Convert(depthImage->GetFormat());
	attachments[1].format = vkFormat;
	attachments[1].samples = Convert(depthImage->GetMSAAsamples());
	attachments[1].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	attachments[1].storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	attachments[1].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	attachments[1].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	attachments[1].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	attachments[1].finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	attachmentRefs[1] = { .attachment = 1, .layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL };

	// Swapchain
	const auto swapchainImage = descAttachments[2];
	vkFormat = Convert(swapchainImage->GetFormat());
	attachments[2].format = vkFormat;
	attachments[2].samples = Convert(swapchainImage->GetMSAAsamples());
	attachments[2].loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	attachments[2].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	attachments[2].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	attachments[2].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	attachments[2].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	attachments[2].finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

	attachmentRefs[2] = { .attachment = 2, .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL };

	std::array<VkSubpassDescription, 1> subpasses{};

	subpasses[0].pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpasses[0].colorAttachmentCount = 1;
	subpasses[0].pColorAttachments = &attachmentRefs[0];
	subpasses[0].pDepthStencilAttachment = &attachmentRefs[1];
	subpasses[0].pResolveAttachments = &attachmentRefs[2];

	std::array<VkSubpassDependency, 1> dependencies{};

	dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
	dependencies[0].dstSubpass = 0;
	dependencies[0].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependencies[0].srcAccessMask = 0;
	dependencies[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependencies[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

	VkRenderPassCreateInfo renderPassInfo;
	ZeroInitVkStruct(renderPassInfo, VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO);

	renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
	renderPassInfo.pAttachments = attachments.data();
	renderPassInfo.subpassCount = static_cast<uint32_t>(subpasses.size());
	renderPassInfo.pSubpasses = subpasses.data();
	renderPassInfo.dependencyCount = static_cast<uint32_t>(dependencies.size());
	renderPassInfo.pDependencies = dependencies.data();

	VkResult result = vkCreateRenderPass(Context::GetDevice().GetHandle(), &renderPassInfo, nullptr, &Handle::GetHandle());
	VK_CHECK_RESULT(result);
	ASSERT(Handle::GetHandle(), "RenderPass creation failed");
}
