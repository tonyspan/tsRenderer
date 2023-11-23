#include "Buffer.h"

#include "Context.h"
#include "Device.h"
#include "Vertex.h"

#include "Log.h"

#include <vulkan/vulkan.h>

BufferDescription::BufferDescription()
	: Size(0)
	, Usage(VK_BUFFER_USAGE_FLAG_BITS_MAX_ENUM)
	, Properties(VK_MEMORY_PROPERTY_FLAG_BITS_MAX_ENUM)
{
}

BufferDescription::BufferDescription(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags props)
	: Size(size), Usage(usage), Properties(props)
{
}

Ref<Buffer> Buffer::Create()
{
	return CreateRef<Buffer>();
}

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

Ref<Buffer> Buffer::CreateIndex(VkDeviceSize size, const void* data)
{
	BufferDescription desc;
	desc.Size = size;
	desc.Usage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
	desc.Properties = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;

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

	vkDestroyBuffer(device, Handle::GetHandle(), nullptr);
	vkFreeMemory(device, m_Memory, nullptr);
}

void Buffer::Init(const BufferDescription& desc)
{
	m_Description = desc;
	CreateBuffer();
}

void Buffer::SetData(const void* data, VkDeviceSize size, VkDeviceSize offset)
{
	ASSERT(data);
	ASSERT(size <= m_Description.Size);
	ASSERT(m_IsInitialized, "Buffer isn't initialized");

	const auto& device = Context::GetDevice().GetHandle();

	void* mappedData = nullptr;

	vkMapMemory(device, m_Memory, 0, m_Description.Size, 0, (void**)&mappedData);
	memcpy(static_cast<char*>(mappedData) + offset, data, size);
	vkUnmapMemory(device, m_Memory);
}

VkDeviceMemory Buffer::GetMemory() const
{
	return m_Memory;
}

VkDeviceMemory Buffer::GetMemory()
{
	return m_Memory;
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

	VkResult result = vkCreateBuffer(vkDevice, &bufferInfo, nullptr, &Handle::GetHandle());
	VK_CHECK_RESULT(result);
	ASSERT(Handle::GetHandle(), "Buffer Creation failed");

	VkMemoryRequirements memRequirements;
	vkGetBufferMemoryRequirements(vkDevice, Handle::GetHandle(), &memRequirements);

	VkMemoryAllocateInfo allocInfo;
	ZeroInitVkStruct(allocInfo, VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO);

	allocInfo.allocationSize = memRequirements.size;
	allocInfo.memoryTypeIndex = device.GetPhysicalDevice().GetMemoryType(memRequirements.memoryTypeBits, m_Description.Properties);

	result = vkAllocateMemory(vkDevice, &allocInfo, nullptr, &m_Memory);
	VK_CHECK_RESULT(result);
	ASSERT(m_Memory, "Failed to allocate buffer memory");

	result = vkBindBufferMemory(vkDevice, Handle::GetHandle(), m_Memory, 0);
	VK_CHECK_RESULT(result);
}
