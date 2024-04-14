#include "DescriptorPool.h"

#include "Device.h"

#include "Log.h"

#include <volk.h>
#include <vulkan/vulkan.h>

#include <array>

DescriptorPool::DescriptorPool(const Device& device)
	: m_Device(device)
{
	CreateDescriptorPool();
}

DescriptorPool::~DescriptorPool()
{
	Destroy();
}

void DescriptorPool::CreateDescriptorPool()
{
	// Taken from ImGui example
	const std::array poolSizes =
	{
		VkDescriptorPoolSize{ VK_DESCRIPTOR_TYPE_SAMPLER,                1000 },
		VkDescriptorPoolSize{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000 },
		VkDescriptorPoolSize{ VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,          1000 },
		VkDescriptorPoolSize{ VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,          1000 },
		VkDescriptorPoolSize{ VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER,   1000 },
		VkDescriptorPoolSize{ VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER,   1000 },
		VkDescriptorPoolSize{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,         1000 },
		VkDescriptorPoolSize{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,         1000 },
		VkDescriptorPoolSize{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000 },
		VkDescriptorPoolSize{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000 },
		VkDescriptorPoolSize{ VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT,       1000 }
	};

	VkDescriptorPoolCreateInfo poolInfo;
	ZeroInitVkStruct(poolInfo, VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO);

	poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
	poolInfo.pPoolSizes = poolSizes.data();
	poolInfo.maxSets = 1000 * static_cast<uint32_t>(poolSizes.size());

	VkResult result = vkCreateDescriptorPool(m_Device.GetHandle(), &poolInfo, nullptr, &Handle::GetHandle());
	VK_CHECK_RESULT(result);
	ASSERT(Handle::GetHandle(), "DescriptorPool creation failed");
}

void DescriptorPool::Destroy()
{
	vkDestroyDescriptorPool(m_Device.GetHandle(), Handle::GetHandle(), nullptr);
}
