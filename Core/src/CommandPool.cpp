#include "CommandPool.h"

#include "Device.h"

#include "Log.h"

#include <vulkan/vulkan.h>

CommandPool::CommandPool(const Device& device)
	: m_Device(device)
{
	CreateCommandPool();
}

CommandPool::~CommandPool()
{
	vkDestroyCommandPool(m_Device.GetHandle(), Handle::GetHandle(), nullptr);
}

void CommandPool::CreateCommandPool()
{
	VkCommandPoolCreateInfo poolInfo;
	ZeroInitVkStruct(poolInfo, VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO);

	poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
	poolInfo.queueFamilyIndex = m_Device.GetPhysicalDevice().GetQueueFamilyIndices().GraphicsIndex.value();

	VkResult result = vkCreateCommandPool(m_Device.GetHandle(), &poolInfo, nullptr, &Handle::GetHandle());
	VK_CHECK_RESULT(result);
	ASSERT(Handle::GetHandle(), "CommandPool creation failed");
}
