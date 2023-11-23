#include "SwapchainSupportDetails.h"

#include <vulkan/vulkan.h>

SwapchainSupportDetails::SwapchainSupportDetails()
{
	Capabilities = new VkSurfaceCapabilitiesKHR();
}

SwapchainSupportDetails::SwapchainSupportDetails(const SwapchainSupportDetails& other)
{
	Capabilities = new VkSurfaceCapabilitiesKHR();

	*Capabilities = *other.Capabilities;
	Formats = other.Formats;
	PresentModes = other.PresentModes;
}

SwapchainSupportDetails::~SwapchainSupportDetails()
{
	delete Capabilities;
}

SwapchainSupportDetails SwapchainSupportDetails::Query(const VkPhysicalDevice device, const VkSurfaceKHR surface)
{
	SwapchainSupportDetails details;

	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, details.Capabilities);

	uint32_t formatCount;
	vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, nullptr);

	if (formatCount != 0)
	{
		details.Formats.resize(formatCount);
		vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, details.Formats.data());
	}

	uint32_t presentModeCount;
	vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, nullptr);

	if (presentModeCount != 0)
	{
		details.PresentModes.resize(presentModeCount);
		vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, details.PresentModes.data());
	}

	return details;
}
