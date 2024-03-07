#pragma once

#include "Base.h"

#include "VK.h"

struct BufferDescription
{
	VkDeviceSize Size;
	VkBufferUsageFlags Usage;
	VkMemoryPropertyFlags Properties;

	BufferDescription();
	explicit BufferDescription(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags props);
};

class Buffer : public Handle<VkBuffer>
{
public:
	// Need to call Init()
	static Ref<Buffer> Create();
	static Ref<Buffer> Create(const BufferDescription& desc);

	static Ref<Buffer> CreateVertex(VkDeviceSize size, const void* data = nullptr);
	static Ref<Buffer> CreateIndex(VkDeviceSize size, const void* data = nullptr);
	static Ref<Buffer> CreateUniform(VkDeviceSize size);
	static Scope<Buffer> CreateStaging(VkDeviceSize size);

	Buffer() = default;
	Buffer(const BufferDescription& desc);
	~Buffer();

	DELETE_COPY_AND_MOVE(Buffer);

	void Init(const BufferDescription& desc);

	void SetData(const void* data, VkDeviceSize size, VkDeviceSize offset = 0);
	// Will use the size defined with BufferDescription::Size
	void SetData(const void* data);

	const BufferDescription& GetDescription() const;

	VkDeviceMemory GetMemory() const;
	VkDeviceMemory GetMemory();
private:
	void CreateBuffer();
private:
	VkDeviceMemory m_Memory = nullptr;
	BufferDescription m_Description;

	bool m_IsInitialized = false;
};
