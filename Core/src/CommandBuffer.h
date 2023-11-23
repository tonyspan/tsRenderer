#pragma once

#include "Base.h"

#include "VK.h"

class CommandBuffer;
class CommandPool;
class Buffer;
class Pipeline;

struct CommandBufferDescription
{
	const CommandPool* Pool;
	VkCommandBufferLevel Level;

	CommandBufferDescription();
	CommandBufferDescription(const CommandPool* pool, VkCommandBufferLevel level);
};

class CommandBuffer : public Handle<VkCommandBuffer>
{
public:
	static Ref<CommandBuffer> Create(const CommandBufferDescription& desc);

	CommandBuffer(const CommandBufferDescription& desc);
	~CommandBuffer();

	void BeginRecording();
	void EndRecording();
	void Reset();

	void BeginSingleTime();
	void EndSingleTime();

	void BindDescriptorSet(VkPipelineLayout layout, VkDescriptorSet set);
	void BindVertexBuffer(const Buffer& buffer);
	void BindIndexBuffer(const Buffer& buffer);
	void BindPipeline(const Pipeline& pipeline);

	void Draw(uint32_t vertexCount);
	void DrawIndexed(uint32_t indexCount, uint32_t firstIndex = 0);
private:
	void CreateCommandBuffer();
private:
	CommandBufferDescription m_Description;
};
