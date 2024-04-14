#include "Synchronization.h"

#include "Device.h"

#include "Log.h"

#include <volk.h>
#include <vulkan/vulkan.h>

#pragma region Fence

Ref<Semaphore> Semaphore::Create(const Device& device)
{
	return CreateRef<Semaphore>(device);
}

Semaphore::Semaphore(const Device& device)
	: m_Device(device)
{
	CreateSemaphore();
}

Semaphore::~Semaphore()
{
	vkDestroySemaphore(m_Device.GetHandle(), Handle::GetHandle(), nullptr);
}

void Semaphore::CreateSemaphore()
{
	VkSemaphoreCreateInfo semaphoreInfo;
	ZeroInitVkStruct(semaphoreInfo, VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO);

	vkCreateSemaphore(m_Device.GetHandle(), &semaphoreInfo, nullptr, &Handle::GetHandle());
	ASSERT(Handle::GetHandle(), "Semaphore creation failed");
}

#pragma endregion

#pragma region Fence

FenceDescription::FenceDescription()
	: CreateFlags(VK_FENCE_CREATE_FLAG_BITS_MAX_ENUM)
{
}

Ref<Fence> Fence::Create(const Device& device, const FenceDescription& desc)
{
	return CreateRef<Fence>(device, desc);
}

Fence::Fence(const Device& device, const FenceDescription& desc)
	: m_Device(device)
{
	CreateFence(desc);
}

Fence::~Fence()
{
	vkDestroyFence(m_Device.GetHandle(), Handle::GetHandle(), nullptr);
}

void Fence::Wait(uint64_t timeout)
{
	vkWaitForFences(m_Device.GetHandle(), 1, &Handle::GetHandle(), VK_TRUE, timeout);
}

void Fence::Reset()
{
	vkResetFences(m_Device.GetHandle(), 1, &Handle::GetHandle());
}

void Fence::CreateFence(const FenceDescription& desc)
{
	VkFenceCreateInfo fenceInfo;
	ZeroInitVkStruct(fenceInfo, VK_STRUCTURE_TYPE_FENCE_CREATE_INFO);

	fenceInfo.flags = desc.CreateFlags;

	vkCreateFence(m_Device.GetHandle(), &fenceInfo, nullptr, &Handle::GetHandle());
	ASSERT(Handle::GetHandle(), "Fence creation failed");
}

#pragma endregion