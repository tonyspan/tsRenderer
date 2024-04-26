#include "RenderPass.h"

#include "Context.h"
#include "Device.h"
#include "Swapchain.h"
#include "CommandBuffer.h"
#include "Framebuffer.h"
#include "Image.h"

#include "Log.h"

#include <volk.h>
#include <vulkan/vulkan.h>

#include <array>

Ref<RenderPass> RenderPass::Create(const RenderPassDescription& desc)
{
	return CreateRef<RenderPass>(desc);
}

RenderPass::RenderPass(const RenderPassDescription& desc)
	: m_Description(desc)
{
	CreateRenderPass();
}

RenderPass::~RenderPass()
{
	vkDestroyRenderPass(Context::GetDevice().GetHandle(), Handle::GetHandle(), nullptr);
}

const RenderPassDescription& RenderPass::GetDescription() const
{
	return m_Description;
}

void RenderPass::CreateRenderPass()
{
	ASSERT(m_Description.MSAAnumSamples.has_value());

	const auto& descAttachments = m_Description.Attachments;

	const bool isMultisampled = m_Description.MSAAnumSamples > 1;
	const auto& msaaSamplesValue = m_Description.MSAAnumSamples.value();
	const auto vkSampleCount = Convert(msaaSamplesValue);

	const size_t size = isMultisampled ? descAttachments.size() : 2;

	std::vector<VkAttachmentDescription> attachments(size, VkAttachmentDescription{});
	std::vector<VkAttachmentReference> attachmentRefs(size, VkAttachmentReference{});

	if (!isMultisampled)
	{
		ASSERT(VK_SAMPLE_COUNT_1_BIT == vkSampleCount);

		// Color/Swapchain
		attachments[0] =
		{
			  .flags = {},
			  .format = Convert(descAttachments[0]->GetFormat()),
			  .samples = vkSampleCount,
			  .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
			  .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
			  .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
			  .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
			  .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
			  .finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR
		};

		attachmentRefs[0] = { .attachment = 0, .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL };

		// Depth
		attachments[1] =
		{
			  .flags = {},
			  .format = Convert(descAttachments[1]->GetFormat()),
			  .samples = vkSampleCount,
			  .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
			  .storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
			  .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
			  .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
			  .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
			  .finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL
		};

		attachmentRefs[1] = { .attachment = 1, .layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL };

		ASSERT(!descAttachments[2], "Must be invalid");
	}
	else
	{
		// Color/Multisampled
		attachments[0] =
		{
			   .flags = {},
			   .format = Convert(descAttachments[0]->GetFormat()),
			   .samples = vkSampleCount,
			   .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
			   .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
			   .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
			   .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
			   .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
			   .finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
		};

		attachmentRefs[0] = { .attachment = 0, .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL };

		// Depth
		attachments[1] =
		{
			.flags = {},
			.format = Convert(descAttachments[1]->GetFormat()),
			.samples = vkSampleCount,
			.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
			.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
			.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
			.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
			.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
			.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL
		};

		attachmentRefs[1] = { .attachment = 1, .layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL };

		// Color/Swapchain
		attachments[2] =
		{
			.flags = {},
			.format = Convert(descAttachments[2]->GetFormat()),
			.samples = VK_SAMPLE_COUNT_1_BIT,
			.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
			.storeOp = VK_ATTACHMENT_STORE_OP_STORE,
			.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
			.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
			.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
			.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR
		};

		attachmentRefs[2] = { .attachment = 2, .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL };
	}

	std::array<VkSubpassDescription, 1> subpasses{};

	subpasses[0].pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpasses[0].colorAttachmentCount = 1;
	subpasses[0].pColorAttachments = &attachmentRefs[0];
	subpasses[0].pDepthStencilAttachment = &attachmentRefs[1];
	subpasses[0].pResolveAttachments = isMultisampled ? &attachmentRefs[2] : nullptr;

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
