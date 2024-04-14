#include "GBuffer.h"

#include "Context.h"
#include "Device.h"

#include "Log.h"

#include <volk.h>
#include <vulkan/vulkan.h>

Ref<GBuffer> GBuffer::Create(const GBufferDescription& desc)
{
	return CreateRef<GBuffer>(desc);
}

Ref<GBuffer> GBuffer::CreateVertex(VkDeviceSize size, const void* data)
{
	GBufferDescription desc;

	desc.Size = size;
	desc.Usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
	desc.Properties = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;

	Ref<GBuffer> buffer = GBuffer::Create(desc);

	if (data)
		buffer->SetData(data, desc.Size);

	return buffer;
}

Ref<GBuffer> GBuffer::CreateIndex(VkDeviceSize size, uint32_t count, const void* data)
{
	GBufferDescription desc;

	desc.Size = size;
	desc.Usage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
	desc.Properties = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
	desc.IndexCount = count;

	Ref<GBuffer> buffer = GBuffer::Create(desc);

	if (data)
		buffer->SetData(data, desc.Size);

	return buffer;
}

Ref<GBuffer> GBuffer::CreateUniform(VkDeviceSize size)
{
	GBufferDescription desc;

	desc.Size = size;
	desc.Usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
	desc.Properties = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;

	return GBuffer::Create(desc);
}

Scope<GBuffer> GBuffer::CreateStaging(VkDeviceSize size)
{
	GBufferDescription desc;

	desc.Size = size;
	desc.Usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
	desc.Properties = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;

	return CreateScope<GBuffer>(desc);
}

GBuffer::GBuffer(const GBufferDescription& desc)
	: m_Description(desc)
{
	CreateBuffer();
}

GBuffer::~GBuffer()
{
	const auto& device = Context::GetDevice().GetHandle();

	vkDestroyBuffer(device, Handle::GetHandle<VkBuffer>(), nullptr);
	vkFreeMemory(device, Handle::GetHandle<VkDeviceMemory>(), nullptr);
}

void GBuffer::SetData(const void* data, VkDeviceSize size, VkDeviceSize offset)
{
	ASSERT(data);
	ASSERT(offset + size <= m_Description.Size);

	const auto& device = Context::GetDevice().GetHandle();
	auto& memoryHandle = Handle::GetHandle<VkDeviceMemory>();

	void* mappedData = nullptr;

	vkMapMemory(device, memoryHandle, 0, size, 0, (void**)&mappedData);
	memcpy(static_cast<char*>(mappedData) + offset, data, static_cast<size_t>(size));
	vkUnmapMemory(device, memoryHandle);
}

void GBuffer::SetData(const void* data)
{
	SetData(data, m_Description.Size);
}

const GBufferDescription& GBuffer::GetDescription() const
{
	return m_Description;
}

void GBuffer::CreateBuffer()
{
	const auto& device = Context::GetDevice();
	const auto& vkDevice = device.GetHandle();

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
	ASSERT(bufferHandle, "GBuffer Creation failed");

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
