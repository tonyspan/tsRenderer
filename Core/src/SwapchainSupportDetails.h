#pragma once

#include "VK.h"

#include <vector>

struct SwapchainSupportDetails
{
	VkSurfaceCapabilitiesKHR* Capabilities;
	std::vector<VkSurfaceFormatKHR> Formats;
	std::vector<VkPresentModeKHR> PresentModes;

	static SwapchainSupportDetails Query(const VkPhysicalDevice device, const VkSurfaceKHR surface);

	SwapchainSupportDetails();
	SwapchainSupportDetails(const SwapchainSupportDetails& other);
	~SwapchainSupportDetails();

	bool IsAdequate() const { return !Formats.empty() && !PresentModes.empty(); }
};

