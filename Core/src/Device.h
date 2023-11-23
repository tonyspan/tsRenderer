#pragma once

#include "Base.h"

#include "VK.h"

#include "CommandPool.h"

#include <optional>

class Instance;
class Surface;

struct QueueFamilyIndices
{
	std::optional<uint32_t> GraphicsIndex;
	std::optional<uint32_t> PresentIndex;

	bool IsComplete() const { return GraphicsIndex.has_value() && PresentIndex.has_value(); }

	bool IsSame() const { return GraphicsIndex == PresentIndex; }
};

class PhysicalDevice : public Handle<VkPhysicalDevice>
{
public:
	PhysicalDevice(const Instance& instance);
	~PhysicalDevice();

	void Select(const Surface& surface);

	const VkPhysicalDeviceProperties& GetProperties() const;
	VkSampleCountFlagBits GetMsaaSamples() const;
	const QueueFamilyIndices& GetQueueFamilyIndices() const;

	VkFormat GetDepthFormat() const;
	uint32_t GetMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties) const;
private:
	const Instance& m_Instance;

	QueueFamilyIndices m_QueueFamilyIndices;

	VkPhysicalDeviceProperties* m_Properties;
};

class Device : public Handle<VkDevice>
{
public:
	Device(const Instance& instance, const Surface& surface);
	~Device();

	void WaitIdle();

	const PhysicalDevice& GetPhysicalDevice() const;
	VkQueue GetGraphicsQueue() const;
	VkQueue GetPresentQueue() const;
	const CommandPool& GetCommandPool() const;
private:
	void CreateDeviceAndQueues();
private:
	PhysicalDevice m_PhysicalDevice;
	VkQueue m_GraphicsQueue;
	VkQueue m_PresentQueue;

	// Should it be here?
	Scope<CommandPool> m_CommandPool;
};