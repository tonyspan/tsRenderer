#include "CommandBuffer.h"

#include "Context.h"
#include "Device.h"
#include "GBuffer.h"
#include "DescriptorSet.h"
#include "Pipeline.h"
#include "Shader.h"
#include "RenderPass.h"
#include "Framebuffer.h"

#include "Log.h"

#include <volk.h>
#include <vulkan/vulkan.h>

#include <glm/glm.hpp>

#include <array>

template<typename T>
static void PushConstants(VkCommandBuffer cmdBuffer, const Pipeline* pipeline, const std::string& name, const T& value)
{
	ASSERT(cmdBuffer);
	ASSERT(pipeline);

	const auto layout = pipeline->GetHandle<VkPipelineLayout>();

	const auto shader = pipeline->GetShader().lock();
	assert(shader);

	const auto pc = shader->TryGetPushConstant(name);

	if (layout && pc)
	{
		ASSERT(sizeof(value) == pc->Size);
		vkCmdPushConstants(cmdBuffer, layout, pc->Stage, pc->Offset, pc->Size, &value);
	}
}

Ref<CommandBuffer> CommandBuffer::Create(bool isPrimary)
{
	return CreateRef<CommandBuffer>(isPrimary);
}

CommandBuffer::CommandBuffer(bool isPrimary)
{
	CreateCommandBuffer(isPrimary);
}

CommandBuffer::~CommandBuffer()
{
	vkFreeCommandBuffers(Context::GetDevice().GetHandle(), Context::GetDevice().GetCommandPool().GetHandle(), 1, &Handle::GetHandle());
}

void CommandBuffer::BeginRecording(bool singleTime)
{
	VkCommandBufferBeginInfo beginInfo;
	ZeroInitVkStruct(beginInfo, VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO);

	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;

	if (singleTime)
		beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

	VkResult result = vkBeginCommandBuffer(Handle::GetHandle(), &beginInfo);
	VK_CHECK_RESULT(result);
}

void CommandBuffer::EndRecording()
{
	VkResult result = vkEndCommandBuffer(Handle::GetHandle());
	VK_CHECK_RESULT(result);
}

void CommandBuffer::Submit()
{
	const auto& graphicsQueue = Context::GetDevice().GetGraphicsQueue();

	VkSubmitInfo submitInfo;
	ZeroInitVkStruct(submitInfo, VK_STRUCTURE_TYPE_SUBMIT_INFO);

	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &Handle::GetHandle();

	vkQueueSubmit(graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
	vkQueueWaitIdle(graphicsQueue);
}

void CommandBuffer::EndRecordingAndSubmit()
{
	EndRecording();
	Submit();
}

void CommandBuffer::BeginRenderPass(const RenderPass& renderPass, const Framebuffer& framebuffer)
{
	VkRenderPassBeginInfo renderPassInfo;
	ZeroInitVkStruct(renderPassInfo, VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO);

	const auto& framebufferDesc = framebuffer.GetDescription();
	const auto& clearColor = framebufferDesc.ClearColor;

	renderPassInfo.renderPass = renderPass.GetHandle();
	renderPassInfo.framebuffer = framebuffer.GetHandle();
	renderPassInfo.renderArea.offset = { 0, 0 };
	renderPassInfo.renderArea.extent = { .width = framebufferDesc.Width, .height = framebufferDesc.Height };

	std::array<VkClearValue, 2> clearValues{};
	clearValues[0].color = { { clearColor.r, clearColor.g, clearColor.b, clearColor.a } };
	clearValues[1].depthStencil = { 1.0f, 0 };

	renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
	renderPassInfo.pClearValues = clearValues.data();

	vkCmdBeginRenderPass(Handle::GetHandle(), &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
}

void CommandBuffer::EndRenderPass()
{
	vkCmdEndRenderPass(Handle::GetHandle());
}

void CommandBuffer::Reset()
{
	vkResetCommandBuffer(Handle::GetHandle(), 0);
}

void CommandBuffer::SetViewport(uint32_t width, uint32_t height, uint32_t x, uint32_t y)
{
	ASSERT(m_BoundPipeline, "Pipeline must be valid");

	if (!m_BoundPipeline->GetDescription().EnableDynamicStates)
		LOG(STR(PipelineDescription::EnableDynamicStates) " is not enabled");

	VkViewport viewport = {};

	viewport.x = float(x);
	viewport.y = float(y);
	viewport.width = float(width);
	viewport.height = float(height);
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;

	vkCmdSetViewport(Handle::GetHandle(), 0, 1, &viewport);

	VkRect2D scissorRect = {};
	scissorRect.offset = { int32_t(x), int32_t(y) };
	scissorRect.extent = { width, height };

	vkCmdSetScissor(Handle::GetHandle(), 0, 1, &scissorRect);
}

void CommandBuffer::SetLineWidth(float lineWidth)
{
	ASSERT(m_BoundPipeline, "Pipeline must be valid");

	if (!m_BoundPipeline->GetDescription().EnableDynamicStates)
		LOG(STR(PipelineDescription::EnableDynamicStates) " is not enabled");

	vkCmdSetLineWidth(Handle::GetHandle(), lineWidth);
}

void CommandBuffer::BindDescriptorSet(const DescriptorSet& set)
{
	ASSERT(m_BoundPipeline);

	const auto& descSetHandle = set.GetDescriptorSet();
	ASSERT(descSetHandle);

	vkCmdBindDescriptorSets(Handle::GetHandle(), VK_PIPELINE_BIND_POINT_GRAPHICS, m_BoundPipeline->GetHandle<VkPipelineLayout>(), 0, 1, &descSetHandle, 0, nullptr);
}

void CommandBuffer::BindVertexBuffer(const GBuffer& buffer)
{
	const auto& bufferHandle = buffer.GetHandle<VkBuffer>();
	ASSERT(bufferHandle);

	std::array vertexBuffers = { bufferHandle };
	VkDeviceSize offsets = 0;
	vkCmdBindVertexBuffers(Handle::GetHandle(), 0, static_cast<uint32_t>(vertexBuffers.size()), vertexBuffers.data(), &offsets);
}

void CommandBuffer::BindIndexBuffer(const GBuffer& buffer)
{
	const auto& bufferHandle = buffer.GetHandle<VkBuffer>();
	ASSERT(bufferHandle);

	vkCmdBindIndexBuffer(Handle::GetHandle(), bufferHandle, 0, VK_INDEX_TYPE_UINT32);
}

void CommandBuffer::BindPipeline(const Pipeline& pipeline)
{
	const auto& pipelineHandle = pipeline.GetHandle<VkPipeline>();
	ASSERT(pipelineHandle);

	m_BoundPipeline = &pipeline;

	vkCmdBindPipeline(Handle::GetHandle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineHandle);
}

void CommandBuffer::Draw(uint32_t vertexCount, uint32_t firstIndex)
{
	vkCmdDraw(Handle::GetHandle(), vertexCount, 1, firstIndex, 0);
}

void CommandBuffer::DrawIndexed(uint32_t indexCount, uint32_t firstIndex)
{
	vkCmdDrawIndexed(Handle::GetHandle(), indexCount, 1, firstIndex, 0, 0);
}

void CommandBuffer::CreateCommandBuffer(bool isPrimary)
{
	const auto& device = Context::GetDevice();

	VkCommandBufferAllocateInfo bufferAllocInfo;
	ZeroInitVkStruct(bufferAllocInfo, VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO);

	bufferAllocInfo.commandBufferCount = 1;
	bufferAllocInfo.commandPool = device.GetCommandPool().GetHandle();
	bufferAllocInfo.level = isPrimary ? VK_COMMAND_BUFFER_LEVEL_PRIMARY : VK_COMMAND_BUFFER_LEVEL_SECONDARY;

	VkResult result = vkAllocateCommandBuffers(device.GetHandle(), &bufferAllocInfo, &Handle::GetHandle());
	VK_CHECK_RESULT(result);
	ASSERT(Handle::GetHandle(), "Command buffer allocation failed");
}

#define PUSH_CONSTANT_SPECIALIZATION(TYPE) \
template<> \
void CommandBuffer::PushConstant<TYPE>(const std::string& name, const TYPE& value) \
{ \
	PushConstants(Handle::GetHandle(), m_BoundPipeline, name, value); \
}

PUSH_CONSTANT_SPECIALIZATION(float)
PUSH_CONSTANT_SPECIALIZATION(glm::mat4)
PUSH_CONSTANT_SPECIALIZATION(glm::vec3)