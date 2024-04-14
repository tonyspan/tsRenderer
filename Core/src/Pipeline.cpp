#include "Pipeline.h"

#include "Context.h"
#include "Device.h"
#include "Swapchain.h"
#include "RenderPass.h"
#include "DescriptorSet.h"
#include "Shader.h"
#include "Vertex.h"
#include "Layout.h"
#include "Framebuffer.h"

#include "Log.h"

#include <volk.h>
#include <vulkan/vulkan.h>

#include <array>

Ref<Pipeline> Pipeline::Create(const PipelineDescription& desc)
{
	return CreateRef<Pipeline>(desc);
}

Pipeline::Pipeline(const PipelineDescription& desc)
{
	CreatePipeline(desc);
}

Pipeline::~Pipeline()
{
	const auto& device = Context::GetDevice().GetHandle();

	vkDestroyPipelineLayout(device, Handle::GetHandle<VkPipelineLayout>(), nullptr);
	vkDestroyPipeline(device, Handle::GetHandle<VkPipeline>(), nullptr);
}

void Pipeline::CreatePipeline(const PipelineDescription& desc)
{
	const auto& vkDevice = Context::GetDevice().GetHandle();
	const auto& swapchain = Context::GetSwapchain();
	const auto& msaaSamples = swapchain.GetRenderPass()->GetDescription().MSAAnumSamples;
	const auto& swapchainDesc = swapchain.GetDescription();
	const auto& renderPass = Context::GetSwapchain().GetRenderPass();

#define OLD_WAY 0
#if OLD_WAY
	const auto& bindingDescription = Vertex::GetBindingDescription();
	const auto& attributeDescriptions = Vertex::GetAttributeDescriptions();
#else
	const auto bufferLayout = desc.BufferLayout;

	//ASSERT(bufferLayout, "BufferLayout must be valid");

	VkVertexInputBindingDescription bindingDescription = {};
	std::vector<VkVertexInputAttributeDescription> attributeDescriptions;

	if (bufferLayout)
	{
		const auto& bufferLayoutElements = bufferLayout->GetElements();

		bindingDescription.binding = 0;
		bindingDescription.stride = bufferLayout->GetStride();
		bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

		attributeDescriptions.reserve(bufferLayoutElements.size());
		for (uint32_t i = 0; auto & elem : bufferLayoutElements)
		{
			VkVertexInputAttributeDescription attributeDescription = {};

			attributeDescription.binding = 0;
			attributeDescription.location = i;
			attributeDescription.format = Convert(elem.Format);
			attributeDescription.offset = elem.Offset;

			attributeDescriptions.push_back(attributeDescription);

			i++;
		}
	}
#endif

	VkPipelineVertexInputStateCreateInfo vertexInputInfo;
	ZeroInitVkStruct(vertexInputInfo, VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO);

	vertexInputInfo.vertexBindingDescriptionCount = 1;
	vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
	vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
	vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();

	VkPipelineInputAssemblyStateCreateInfo inputAssembly;
	ZeroInitVkStruct(inputAssembly, VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO);

	inputAssembly.topology = Convert(desc.Topology);
	inputAssembly.primitiveRestartEnable = VK_FALSE;

	VkViewport viewport = {};
	viewport.x = 0.0f;
	viewport.y = 0.0f;
	viewport.width = static_cast<float>(swapchainDesc.Width);
	viewport.height = static_cast<float>(swapchainDesc.Height);
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;

	VkRect2D scissor = {};
	scissor.offset = { 0, 0 };
	scissor.extent = { .width = swapchainDesc.Width, .height = swapchainDesc.Height };

	VkPipelineViewportStateCreateInfo viewportState;
	ZeroInitVkStruct(viewportState, VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO);

	viewportState.viewportCount = 1;
	viewportState.pViewports = &viewport;
	viewportState.scissorCount = 1;
	viewportState.pScissors = &scissor;

	VkPipelineRasterizationStateCreateInfo rasterizer;
	ZeroInitVkStruct(rasterizer, VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO);

	rasterizer.depthClampEnable = VK_FALSE;
	rasterizer.rasterizerDiscardEnable = VK_FALSE;
	rasterizer.polygonMode = Convert(desc.PolygonMode);
	rasterizer.lineWidth = 1.0f;
	rasterizer.cullMode = Convert(desc.CullMode);
	rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
	rasterizer.depthBiasEnable = VK_FALSE;

	VkPipelineMultisampleStateCreateInfo multisampling;
	ZeroInitVkStruct(multisampling, VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO);

	ASSERT(msaaSamples.has_value());

	multisampling.sampleShadingEnable = VK_TRUE;
	multisampling.minSampleShading = 0.2f;
	multisampling.rasterizationSamples = msaaSamples > 1 ? Convert(msaaSamples.value()) : VK_SAMPLE_COUNT_1_BIT;

	const bool isTransparencyEnabled = desc.EnableTransparency;

	VkPipelineColorBlendAttachmentState colorBlendAttachment = {};

	colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
	colorBlendAttachment.blendEnable = isTransparencyEnabled ? VK_TRUE : VK_FALSE;
	colorBlendAttachment.srcColorBlendFactor = isTransparencyEnabled ? VK_BLEND_FACTOR_SRC_ALPHA : VK_BLEND_FACTOR_ONE;
	colorBlendAttachment.dstColorBlendFactor = isTransparencyEnabled ? VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA : VK_BLEND_FACTOR_ZERO;
	colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
	colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
	colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
	colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;

	VkPipelineColorBlendStateCreateInfo colorBlending;
	ZeroInitVkStruct(colorBlending, VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO);

	colorBlending.logicOpEnable = VK_FALSE;
	colorBlending.attachmentCount = 1;
	colorBlending.pAttachments = &colorBlendAttachment;

	VkPipelineLayoutCreateInfo pipelineLayoutInfo;
	ZeroInitVkStruct(pipelineLayoutInfo, VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO);

	if (desc.DescSetLayout)
	{
		std::array<VkDescriptorSetLayout, 1> descriptorSetLayout = { desc.DescSetLayout };

		pipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(descriptorSetLayout.size());
		pipelineLayoutInfo.pSetLayouts = descriptorSetLayout.data();
	}

	auto& pipelineLayoutHandle = Handle::GetHandle<VkPipelineLayout>();

	VkResult result = vkCreatePipelineLayout(vkDevice, &pipelineLayoutInfo, nullptr, &pipelineLayoutHandle);
	VK_CHECK_RESULT(result);
	ASSERT(pipelineLayoutHandle, "Pipeline layout creation failed");

	VkPipelineDepthStencilStateCreateInfo depthStencil;
	ZeroInitVkStruct(depthStencil, VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO);

	depthStencil.depthTestEnable = VK_TRUE;
	depthStencil.depthWriteEnable = VK_TRUE;
	depthStencil.depthCompareOp = Convert(desc.CompareOp);
	depthStencil.depthBoundsTestEnable = VK_FALSE;
	depthStencil.minDepthBounds = 0.0f;
	depthStencil.maxDepthBounds = 1.0f;
	depthStencil.stencilTestEnable = VK_FALSE;

	std::vector<Ref<Shader>> shaders(desc.ShaderModules.size());
	for (uint32_t i = 0; const auto & [stage, path] : desc.ShaderModules)
		shaders[i++] = Shader::Create(stage, path);

	std::vector<VkPipelineShaderStageCreateInfo> pipelineShaderStageCreateInfos;
	for (const auto& shader : shaders)
	{
		if (shader)
			pipelineShaderStageCreateInfos.emplace_back(shader->GetCreateInfoForPipeline());
	}

	VkGraphicsPipelineCreateInfo pipelineInfo;
	ZeroInitVkStruct(pipelineInfo, VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO);

	pipelineInfo.stageCount = static_cast<uint32_t>(pipelineShaderStageCreateInfos.size());
	pipelineInfo.pStages = pipelineShaderStageCreateInfos.data();
	pipelineInfo.pVertexInputState = &vertexInputInfo;
	pipelineInfo.pInputAssemblyState = &inputAssembly;
	pipelineInfo.pViewportState = &viewportState;
	pipelineInfo.pRasterizationState = &rasterizer;
	pipelineInfo.pMultisampleState = &multisampling;
	pipelineInfo.pDepthStencilState = &depthStencil;
	pipelineInfo.pColorBlendState = &colorBlending;
	pipelineInfo.layout = pipelineLayoutHandle;
	pipelineInfo.renderPass = renderPass->GetHandle();
	pipelineInfo.subpass = 0;

	auto& pipelineHandle = Handle::GetHandle<VkPipeline>();

	result = vkCreateGraphicsPipelines(vkDevice, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &pipelineHandle);
	VK_CHECK_RESULT(result);
	ASSERT(pipelineHandle, "Grapphics pipeline creation failed");

	// Unnecessary
	for (auto& shader : shaders)
		shader.reset();
}
