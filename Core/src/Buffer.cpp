#include "Buffer.h"

#include "Context.h"
#include "Device.h"
#include "Vertex.h"

#include "Log.h"

#include <vulkan/vulkan.h>

Ref<Buffer> Buffer::Create(const BufferDescription& desc)
{
	return CreateRef<Buffer>(desc);
}

Ref<Buffer> Buffer::CreateVertex(VkDeviceSize size, const void* data)
{
	BufferDescription desc;

	desc.Size = size;
	desc.Usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
	desc.Properties = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;

	Ref<Buffer> buffer = Buffer::Create(desc);

	if (data)
		buffer->SetData(data, desc.Size);

	return buffer;
}

Ref<Buffer> Buffer::CreateIndex(VkDeviceSize size, uint32_t count, const void* data)
{
	BufferDescription desc;
	desc.Size = size;
	desc.Usage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
	desc.Properties = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
	desc.IndexCount = count;

	Ref<Buffer> buffer = Buffer::Create(desc);

	if (data)
		buffer->SetData(data, desc.Size);

	return buffer;
}

Ref<Buffer> Buffer::CreateUniform(VkDeviceSize size)
{
	BufferDescription desc;

	desc.Size = size;
	desc.Usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
	desc.Properties = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;

	return Buffer::Create(desc);
}

Scope<Buffer> Buffer::CreateStaging(VkDeviceSize size)
{
	BufferDescription desc;

	desc.Size = size;
	desc.Usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
	desc.Properties = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;

	return CreateScope<Buffer>(desc);
}

Buffer::Buffer(const BufferDescription& desc)
	: m_Description(desc)
{
	CreateBuffer();
}

Buffer::~Buffer()
{
	const auto& device = Context::GetDevice().GetHandle();

	vkDestroyBuffer(device, Handle::GetHandle<VkBuffer>(), nullptr);
	vkFreeMemory(device, Handle::GetHandle<VkDeviceMemory>(), nullptr);
}

void Buffer::Init(const BufferDescription& desc)
{
	m_Description = desc;
	CreateBuffer();
}

void Buffer::SetData(const void* data, VkDeviceSize size, VkDeviceSize offset)
{
	ASSERT(m_IsInitialized, "Buffer isn't initialized");
	ASSERT(data);
	ASSERT(size > 0 && size <= m_Description.Size);

	const auto& device = Context::GetDevice().GetHandle();
	auto& memoryHandle = Handle::GetHandle<VkDeviceMemory>();

	void* mappedData = nullptr;

	vkMapMemory(device, memoryHandle, 0, size, 0, (void**)&mappedData);
	memcpy(static_cast<char*>(mappedData) + offset, data, static_cast<size_t>(size));
	vkUnmapMemory(device, memoryHandle);
}

void Buffer::SetData(const void* data)
{
	SetData(data, m_Description.Size);
}

const BufferDescription& Buffer::GetDescription() const
{
	return m_Description;
}

void Buffer::CreateBuffer()
{
	const auto& device = Context::GetDevice();
	const auto& vkDevice = device.GetHandle();

	m_IsInitialized = true;

	VkBufferCreateInfo bufferInfo;
	ZeroInitVkStruct(bufferInfo, VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO);

	auto& size = m_Description.Size;
	ASSERT(size > 0);

	bufferInfo.size = size;
	bufferInfo.usage = m_Description.Usage;
	bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	auto& bufferHandle = Handle::GetHandle<VkBuffer>();

	VkResult result = vkCreateBuffer(vkDevice, &bufferInfo, nullptr, &bufferHandle);
	VK_CHECK_RESULT(result);
	ASSERT(bufferHandle, "Buffer Creation failed");

	VkMemoryRequirements memRequirements;
	vkGetBufferMemoryRequirements(vkDevice, bufferHandle, &memRequirements);

	VkMemoryAllocateInfo allocInfo;
	ZeroInitVkStruct(allocInfo, VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO);

	allocInfo.allocationSize = memRequirements.size;
	allocInfo.memoryTypeIndex = device.GetPhysicalDevice().GetMemoryType(memRequirements.memoryTypeBits, m_Description.Properties);

	auto& memoryHandle = Handle::GetHandle<VkDeviceMemory>();

	result = vkAllocateMemory(vkDevice, &allocInfo, nullptr, &memoryHandle);
	VK_CHECK_RESULT(result);
	ASSERT(memoryHandle, "Failed to allocate buffer memory");

	result = vkBindBufferMemory(vkDevice, bufferHandle, memoryHandle, 0);
	VK_CHECK_RESULT(result);
}
