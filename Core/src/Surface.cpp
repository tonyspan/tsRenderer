#include "Surface.h"

#include "Instance.h"
#include "Device.h"
#include "Window.h"
#include "SwapchainSupportDetails.h"

#include "Log.h"

#include <vulkan/vulkan.h>

#include <SDL_vulkan.h>

static bool IsPresentModeSupported(const std::vector<VkPresentModeKHR>& supportedModes, const VkPresentModeKHR presentMode)
{
	for (const auto& mode : supportedModes)
	{
		if (mode == presentMode)
			return true;
	}

	return false;
}

Surface::Surface(const Instance& instance, const Window& window)
	: m_Instance(instance)
{
	CreateSurface(window);
}

Surface::~Surface()
{
	vkDestroySurfaceKHR(m_Instance.GetHandle(), Handle::GetHandle(), nullptr);
}

const VkSurfaceCapabilitiesKHR Surface::GetCapabilities(const PhysicalDevice& device) const
{
	// Is this dangerous?
	return *(SwapchainSupportDetails::Query(device.GetHandle(), Handle::GetHandle()).Capabilities);
}

const VkSurfaceFormatKHR Surface::GetFormat(const PhysicalDevice& device) const
{
	const auto& availableFormats = SwapchainSupportDetails::Query(device.GetHandle(), Handle::GetHandle()).Formats;

	for (const auto& availableFormat : availableFormats)
	{
		// ImGui doesn't like SRGB
		// See more: https://github.com/ocornut/imgui/issues/6583
		if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
			return availableFormat;
	}

	return availableFormats.at(0);
}

const VkPresentModeKHR Surface::GetPresentMode(const PhysicalDevice& device, bool vsync) const
{
	const auto& availablePresentModes = SwapchainSupportDetails::Query(device.GetHandle(), Handle::GetHandle()).PresentModes;

	VkPresentModeKHR presentMode = VK_PRESENT_MODE_FIFO_KHR;
	if (!vsync)
	{
		if (IsPresentModeSupported(availablePresentModes, VK_PRESENT_MODE_IMMEDIATE_KHR))
			presentMode = VK_PRESENT_MODE_IMMEDIATE_KHR;
		else
			LOG("Failed to find supported presentation mode");
	}

	return presentMode;
}

void Surface::CreateSurface(const Window& window)
{
	SDL_Vulkan_CreateSurface(window.GetHandle<SDL_Window>(), m_Instance.GetHandle(), &Handle::GetHandle());
}
