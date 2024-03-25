#pragma once

#include "Base.h"

#include "VK.h"

class CommandBuffer;
class Buffer;
class Pipeline;
class DescriptorSet;

class CommandBuffer : public Handle<VkCommandBuffer>
{
public:
	static Ref<CommandBuffer> Create(bool isPrimary);

	CommandBuffer(bool isPrimary);
	~CommandBuffer();

	void BeginRecording();
	void EndRecording();
	void Reset();

	void BeginSingleTime();
	void EndSingleTime();

	void BindDescriptorSet(const DescriptorSet& set);
	void BindVertexBuffer(const Buffer& buffer);
	void BindIndexBuffer(const Buffer& buffer);
	void BindPipeline(const Pipeline& pipeline);

	void Draw(uint32_t vertexCount);
	void DrawIndexed(uint32_t indexCount, uint32_t firstIndex = 0);
private:
	void CreateCommandBuffer(bool isPrimary);
private:
	const Pipeline* m_BoundPipeline = nullptr;
};
