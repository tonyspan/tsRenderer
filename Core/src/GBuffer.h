#pragma once

#include "Base.h"

#include "VK.h"

struct GBufferDescription
{
	VkDeviceSize Size = 0;
	VkBufferUsageFlags Usage = (VkBufferUsageFlags)VK_MAX_VALUE_ENUM;
	VkMemoryPropertyFlags Properties = (VkMemoryPropertyFlags)VK_MAX_VALUE_ENUM;

	// Used only in the IndexBuffer
	// TODO: probably IndexBuffer class is needed
	uint32_t IndexCount = 0;
};

class GBuffer : public Handle<VkBuffer, VkDeviceMemory>
{
public:
	static Ref<GBuffer> Create(const GBufferDescription& desc);

	static Ref<GBuffer> CreateVertex(VkDeviceSize size, const void* data = nullptr);
	static Ref<GBuffer> CreateIndex(VkDeviceSize size, uint32_t count, const void* data = nullptr);
	static Ref<GBuffer> CreateUniform(VkDeviceSize size);
	static Scope<GBuffer> CreateStaging(VkDeviceSize size);

	GBuffer(const GBufferDescription& desc);
	~GBuffer();

	DELETE_COPY_AND_MOVE(GBuffer);

	void SetData(const void* data, VkDeviceSize size, VkDeviceSize offset = 0);
	// Will use the size defined with GBufferDescription::Size
	void SetData(const void* data);

	const GBufferDescription& GetDescription() const;
private:
	void CreateBuffer();
private:
	GBufferDescription m_Description;
};
