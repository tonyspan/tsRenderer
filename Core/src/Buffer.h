#pragma once

#include "Base.h"

#include "VK.h"

struct BufferDescription
{
	VkDeviceSize Size = 0;
	VkBufferUsageFlags Usage = (VkBufferUsageFlags)VK_MAX_VALUE_ENUM;
	VkMemoryPropertyFlags Properties = (VkMemoryPropertyFlags)VK_MAX_VALUE_ENUM;

	// Used only in the IndexBuffer
	// TODO: probably IndexBuffer class is needed
	uint32_t IndexCount = 0;
};

class Buffer : public Handle<VkBuffer, VkDeviceMemory>
{
public:
	static Ref<Buffer> Create(const BufferDescription& desc);

	static Ref<Buffer> CreateVertex(VkDeviceSize size, const void* data = nullptr);
	static Ref<Buffer> CreateIndex(VkDeviceSize size, uint32_t count, const void* data = nullptr);
	static Ref<Buffer> CreateUniform(VkDeviceSize size);
	static Scope<Buffer> CreateStaging(VkDeviceSize size);

	Buffer(const BufferDescription& desc);
	~Buffer();

	DELETE_COPY_AND_MOVE(Buffer);

	void Init(const BufferDescription& desc);

	void SetData(const void* data, VkDeviceSize size, VkDeviceSize offset = 0);
	// Will use the size defined with BufferDescription::Size
	void SetData(const void* data);

	const BufferDescription& GetDescription() const;
private:
	void CreateBuffer();
private:
	BufferDescription m_Description;

	bool m_IsInitialized = false;
};
