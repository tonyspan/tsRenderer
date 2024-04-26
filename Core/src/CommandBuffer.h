#pragma once

#include "Base.h"

#include "VK.h"

#include <string>

class GBuffer;
class Pipeline;
class DescriptorSet;
class RenderPass;
class Framebuffer;

class CommandBuffer : public Handle<VkCommandBuffer>
{
public:
	static Ref<CommandBuffer> Create(bool isPrimary);

	CommandBuffer(bool isPrimary);
	~CommandBuffer();

	void BeginRecording(bool singleTime = false);
	void EndRecording();
	void Submit();
	void EndRecordingAndSubmit();

	void BeginRenderPass(const RenderPass& renderPass, const Framebuffer& framebuffer);
	void EndRenderPass();

	void Reset();

	void SetViewport(uint32_t width, uint32_t height, uint32_t x = 0, uint32_t y = 0);
	void SetLineWidth(float lineWidth);

	void BindDescriptorSet(const DescriptorSet& set);
	void BindVertexBuffer(const GBuffer& buffer);
	void BindIndexBuffer(const GBuffer& buffer);
	void BindPipeline(const Pipeline& pipeline);

	void Draw(uint32_t vertexCount, uint32_t firstIndex = 0);
	void DrawIndexed(uint32_t indexCount, uint32_t firstIndex = 0);

	template<typename T>
	void PushConstant(const std::string& name, const T& value);
private:
	void CreateCommandBuffer(bool isPrimary);
private:
	const Pipeline* m_BoundPipeline = nullptr;
};
