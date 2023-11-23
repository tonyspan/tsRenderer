#include "CommandBuffer.h"

#include "Context.h"
#include "Device.h"
#include "Buffer.h"
#include "DescriptorSet.h"
#include "Pipeline.h"

#include "Log.h"

#include <vulkan/vulkan.h>

#include <array>

CommandBufferDescription::CommandBufferDescription()
	: Pool(nullptr), Level(VK_COMMAND_BUFFER_LEVEL_PRIMARY)
{
}

CommandBufferDescription::CommandBufferDescription(const CommandPool* pool, VkCommandBufferLevel level)
	: Pool(pool), Level(level)
{
}

Ref<CommandBuffer> CommandBuffer::Create(const CommandBufferDescription& desc)
{
	ASSERT(desc.Pool, "CommandPool must be valid");

	return CreateRef<CommandBuffer>(desc);
}

CommandBuffer::CommandBuffer(const CommandBufferDescription& desc)
	: m_Description(desc)
{
	CreateCommandBuffer();
}

CommandBuffer::~CommandBuffer()
{
	vkFreeCommandBuffers(Context::GetDevice().GetHandle(), m_Description.Pool->GetHandle(), 1, &Handle::GetHandle());
}

void CommandBuffer::BeginRecording()
{
	VkCommandBufferBeginInfo beginInfo;
	ZeroInitVkStruct(beginInfo, VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO);

	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;

	VkResult result = vkBeginCommandBuffer(Handle::GetHandle(), &beginInfo);
	VK_CHECK_RESULT(result);
}

void CommandBuffer::EndRecording()
{
	VkResult result = vkEndCommandBuffer(Handle::GetHandle());
	VK_CHECK_RESULT(result);
}

void CommandBuffer::Reset()
{
	vkResetCommandBuffer(Handle::GetHandle(), 0);
}

void CommandBuffer::BeginSingleTime()
{
	VkCommandBufferBeginInfo beginInfo;
	ZeroInitVkStruct(beginInfo, VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO);

	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

	vkBeginCommandBuffer(Handle::GetHandle(), &beginInfo);
}

void CommandBuffer::EndSingleTime()
{
	const auto& graphicsQueue = Context::GetDevice().GetGraphicsQueue();

	vkEndCommandBuffer(Handle::GetHandle());

	VkSubmitInfo submitInfo;
	ZeroInitVkStruct(submitInfo, VK_STRUCTURE_TYPE_SUBMIT_INFO);

	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &Handle::GetHandle();

	vkQueueSubmit(graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
	vkQueueWaitIdle(graphicsQueue);
}

void CommandBuffer::BindDescriptorSet(VkPipelineLayout layout, VkDescriptorSet set)
{
	vkCmdBindDescriptorSets(Handle::GetHandle(), VK_PIPELINE_BIND_POINT_GRAPHICS, layout, 0, 1, &set, 0, nullptr);
}

void CommandBuffer::BindVertexBuffer(const Buffer& buffer)
{
	std::array vertexBuffers = { buffer.GetHandle() };
	VkDeviceSize offsets = 0;
	vkCmdBindVertexBuffers(Handle::GetHandle(), 0, static_cast<uint32_t>(vertexBuffers.size()), vertexBuffers.data(), &offsets);
}

void CommandBuffer::BindIndexBuffer(const Buffer& buffer)
{
	vkCmdBindIndexBuffer(Handle::GetHandle(), buffer.GetHandle(), 0, VK_INDEX_TYPE_UINT32);
}

void CommandBuffer::BindPipeline(const Pipeline& pipeline)
{
	vkCmdBindPipeline(Handle::GetHandle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.GetHandle());
}

void CommandBuffer::Draw(uint32_t vertexCount)
{
	vkCmdDraw(Handle::GetHandle(), vertexCount, 1, 0, 0);
}

void CommandBuffer::DrawIndexed(uint32_t indexCount, uint32_t firstIndex)
{
	vkCmdDrawIndexed(Handle::GetHandle(), indexCount, 1, firstIndex, 0, 0);
}

void CommandBuffer::CreateCommandBuffer()
{
	const CommandPool* pool = m_Description.Pool;

	VkCommandBufferAllocateInfo bufferAllocInfo;
	ZeroInitVkStruct(bufferAllocInfo, VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO);

	bufferAllocInfo.commandBufferCount = 1;
	bufferAllocInfo.commandPool = pool->GetHandle();
	bufferAllocInfo.level = m_Description.Level;

	VkResult result = vkAllocateCommandBuffers(Context::GetDevice().GetHandle(), &bufferAllocInfo, &Handle::GetHandle());
	VK_CHECK_RESULT(result);
	ASSERT(Handle::GetHandle(), "Command buffer allocation failed");
}
