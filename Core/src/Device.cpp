#include "Device.h"

#include "Instance.h"
#include "Surface.h"
#include "SwapchainSupportDetails.h"
#include "Context.h"
#include "DescriptorPool.h"

#include "Log.h"

#include <volk.h>
#include <vulkan/vulkan.h>

#include <vector>
#include <set>
#include <string>

extern std::vector<const char*> g_ValidationLayers;

static constexpr const char* s_LogTag = "[Device]";

static const std::vector<const char*> s_DeviceExtensions = {
	VK_KHR_SWAPCHAIN_EXTENSION_NAME
};

static QueueFamilyIndices FindQueueFamilies(const VkPhysicalDevice device, const VkSurfaceKHR surface)
{
	QueueFamilyIndices indices;

	uint32_t count;
	vkGetPhysicalDeviceQueueFamilyProperties(device, &count, nullptr);

	std::vector<VkQueueFamilyProperties> queueFamiliesProperties(count);
	vkGetPhysicalDeviceQueueFamilyProperties(device, &count, queueFamiliesProperties.data());

	for (uint32_t i = 0; const VkQueueFamilyProperties & property : queueFamiliesProperties)
	{
		if (property.queueCount > 0 && property.queueFlags & VK_QUEUE_GRAPHICS_BIT)
			indices.GraphicsIndex = i;

		VkBool32 presentSupport = false;
		vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presentSupport);

		if (property.queueCount > 0 && presentSupport)
			indices.PresentIndex = i;

		if (indices.IsComplete())
			break;

		i++;
	}

	return indices;
}

static bool CheckDeviceExtensionSupport(VkPhysicalDevice device)
{
	uint32_t count;
	VkResult result = vkEnumerateDeviceExtensionProperties(device, nullptr, &count, nullptr);
	VK_CHECK_RESULT(result);

	std::vector<VkExtensionProperties> availableExtensions(count);
	result = vkEnumerateDeviceExtensionProperties(device, nullptr, &count, availableExtensions.data());
	VK_CHECK_RESULT(result);

	std::set<std::string> requiredExtensions(s_DeviceExtensions.begin(), s_DeviceExtensions.end());

	for (const auto& extension : availableExtensions)
		requiredExtensions.erase(extension.extensionName);

	return requiredExtensions.empty();
}

static bool IsDeviceSuitable(const VkPhysicalDevice device, const VkSurfaceKHR surface)
{
	QueueFamilyIndices indices = FindQueueFamilies(device, surface);

	bool isExtensionsSupported = CheckDeviceExtensionSupport(device);

	bool isSwapChainAdequate = false;
	if (isExtensionsSupported)
	{
		const auto& swapChainSupport = SwapchainSupportDetails::Query(device, surface);

		isSwapChainAdequate = swapChainSupport.IsAdequate();
	}

	VkPhysicalDeviceFeatures supportedFeatures;
	vkGetPhysicalDeviceFeatures(device, &supportedFeatures);

	return indices.IsComplete() && isExtensionsSupported && isSwapChainAdequate && supportedFeatures.samplerAnisotropy;
}

static VkSampleCountFlagBits GetMaxUsableSampleCount(const VkPhysicalDeviceProperties physicalDeviceProperties)
{
	VkSampleCountFlags counts =
		physicalDeviceProperties.limits.framebufferColorSampleCounts &
		physicalDeviceProperties.limits.framebufferDepthSampleCounts;

	if (counts & VK_SAMPLE_COUNT_64_BIT)
		return VK_SAMPLE_COUNT_64_BIT;
	if (counts & VK_SAMPLE_COUNT_32_BIT)
		return VK_SAMPLE_COUNT_32_BIT;
	if (counts & VK_SAMPLE_COUNT_16_BIT)
		return VK_SAMPLE_COUNT_16_BIT;
	if (counts & VK_SAMPLE_COUNT_8_BIT)
		return VK_SAMPLE_COUNT_8_BIT;
	if (counts & VK_SAMPLE_COUNT_4_BIT)
		return VK_SAMPLE_COUNT_4_BIT;
	if (counts & VK_SAMPLE_COUNT_2_BIT)
		return VK_SAMPLE_COUNT_2_BIT;

	return VK_SAMPLE_COUNT_1_BIT;
}

PhysicalDevice::PhysicalDevice(const Instance& instance)
	: m_Instance(instance), m_Properties(new VkPhysicalDeviceProperties())
{
}

PhysicalDevice::~PhysicalDevice()
{
	delete m_Properties;
}

void PhysicalDevice::Select(const Surface& surface)
{
	ASSERT(surface.GetHandle());

	uint32_t count = 0;
	vkEnumeratePhysicalDevices(m_Instance.GetHandle(), &count, nullptr);
	ASSERT(count != 0, "Failed to find GPUs with Vulkan support");

	std::vector<VkPhysicalDevice> physicalDevices(count);
	vkEnumeratePhysicalDevices(m_Instance.GetHandle(), &count, physicalDevices.data());

	LOG_TAGGED(s_LogTag, "Physical devices found: %i", count);

	for (const auto& device : physicalDevices)
	{
		if (IsDeviceSuitable(device, surface.GetHandle()))
		{
			Handle::GetHandle() = device;

			vkGetPhysicalDeviceProperties(Handle::GetHandle(), m_Properties);

			m_QueueFamilyIndices = FindQueueFamilies(device, surface.GetHandle());

			break;
		}
	}

	ASSERT(Handle::GetHandle(), "Failed to find a suitable GPU");
	ASSERT(m_QueueFamilyIndices.IsComplete());

	LOG_TAGGED(s_LogTag, "Selected GPU: %s", QUOTED(m_Properties->deviceName));
}

const VkPhysicalDeviceProperties& PhysicalDevice::GetProperties() const
{
	return *m_Properties;
}

VkSampleCountFlagBits PhysicalDevice::GetMsaaSamples() const
{
	return GetMaxUsableSampleCount(*m_Properties);
}

const QueueFamilyIndices& PhysicalDevice::GetQueueFamilyIndices() const
{
	return m_QueueFamilyIndices;
}

VkFormat PhysicalDevice::GetDepthFormat() const
{
	std::vector<VkFormat> candidates = { VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT };
	VkImageTiling tiling = VK_IMAGE_TILING_OPTIMAL;
	VkFormatFeatureFlags features = VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT;

	for (VkFormat format : candidates)
	{
		VkFormatProperties props;

		vkGetPhysicalDeviceFormatProperties(Handle::GetHandle(), format, &props);
		if (tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & features) == features)
			return format;
		else if (tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features) == features)
			return format;
	}

	ASSERT(false, "Failed to find depth format");

	return {};
}

uint32_t PhysicalDevice::GetMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties) const
{
	VkPhysicalDeviceMemoryProperties memProperties;
	vkGetPhysicalDeviceMemoryProperties(Handle::GetHandle(), &memProperties);

	for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++)
	{
		if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties)
			return i;
	}

	ASSERT(false, "Failed to find suitable memory type");

	return {};
}

Device::Device(const Instance& instance, const Surface& surface)
	: m_PhysicalDevice(instance)
{
	m_PhysicalDevice.Select(surface);
	CreateDeviceAndQueues();

	m_CommandPool = CreateScope<CommandPool>(*this);
	m_DescriptorPool = CreateScope<DescriptorPool>(*this);
}

Device::~Device()
{
	m_CommandPool.reset();
	m_DescriptorPool.reset();

	vkDestroyDevice(Handle::GetHandle(), nullptr);
}

void Device::WaitIdle()
{
	vkDeviceWaitIdle(Handle::GetHandle());
}

const PhysicalDevice& Device::GetPhysicalDevice() const
{
	return m_PhysicalDevice;
}

VkQueue Device::GetGraphicsQueue() const
{
	return m_GraphicsQueue;
}

VkQueue Device::GetPresentQueue() const
{
	return m_PresentQueue;
}

const CommandPool& Device::GetCommandPool() const
{
	return *m_CommandPool;
}

const DescriptorPool& Device::GetDescriptorPool() const
{
	return *m_DescriptorPool;
}

void Device::CreateDeviceAndQueues()
{
	constexpr float queuePriority = 1.0f;

	const QueueFamilyIndices& indices = m_PhysicalDevice.GetQueueFamilyIndices();
	const uint32_t& graphicsIndex = indices.GraphicsIndex.value();
	const uint32_t& presentIndex = indices.PresentIndex.value();

	std::set<uint32_t> uniqueIndices = { graphicsIndex, presentIndex };

	std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
	for (uint32_t queueFamily : uniqueIndices)
	{
		VkDeviceQueueCreateInfo queueCreateInfo;
		ZeroInitVkStruct(queueCreateInfo, VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO);

		queueCreateInfo.queueFamilyIndex = queueFamily;
		queueCreateInfo.queueCount = 1;
		queueCreateInfo.pQueuePriorities = &queuePriority;

		queueCreateInfos.emplace_back(queueCreateInfo);
	}

	VkPhysicalDeviceFeatures deviceFeatures = {};
	deviceFeatures.samplerAnisotropy = VK_TRUE;
	deviceFeatures.sampleRateShading = VK_TRUE;
	deviceFeatures.fillModeNonSolid = VK_TRUE;
	deviceFeatures.wideLines = VK_TRUE;

	VkDeviceCreateInfo createInfo;
	ZeroInitVkStruct(createInfo, VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO);

	createInfo.pQueueCreateInfos = queueCreateInfos.data();
	createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());

	createInfo.pEnabledFeatures = &deviceFeatures;
	createInfo.enabledExtensionCount = static_cast<uint32_t>(s_DeviceExtensions.size());
	createInfo.ppEnabledExtensionNames = s_DeviceExtensions.data();

	createInfo.enabledLayerCount = static_cast<uint32_t>(g_ValidationLayers.size());
	createInfo.ppEnabledLayerNames = g_ValidationLayers.data();

	VkResult result = vkCreateDevice(m_PhysicalDevice.GetHandle(), &createInfo, nullptr, &Handle::GetHandle());
	VK_CHECK_RESULT(result);
	ASSERT(Handle::GetHandle(), "Device creation failed");

	vkGetDeviceQueue(Handle::GetHandle(), graphicsIndex, 0, &m_GraphicsQueue);
	vkGetDeviceQueue(Handle::GetHandle(), presentIndex, 0, &m_PresentQueue);
	ASSERT(m_GraphicsQueue && m_PresentQueue, "Queue creation failed");
}
