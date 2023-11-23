#pragma once

#include "Base.h"

#include "VK.h"

class Device;

#pragma region Semaphore

class Semaphore : public Handle<VkSemaphore>
{
public:
	static Ref<Semaphore> Create(const Device& device);

	Semaphore(const Device& device);
	~Semaphore();
private:
	void CreateSemaphore();
private:
	const Device& m_Device;
};

#pragma endregion

#pragma region Fence

struct FenceDescription
{
	VkFenceCreateFlags CreateFlags;

	FenceDescription();
};

class Fence : public Handle<VkFence>
{
public:
	static Ref<Fence> Create(const Device& device, const FenceDescription& desc);

	Fence(const Device& device, const FenceDescription& desc);
	~Fence();

	void Wait(uint64_t timeout);
	void Reset();
private:
	void CreateFence(const FenceDescription& desc);
private:
	const Device& m_Device;
};

#pragma endregion