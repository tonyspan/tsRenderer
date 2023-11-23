#pragma once

#include "VK.h"

class Instance;
class Window;
class PhysicalDevice;

class Surface : public Handle<VkSurfaceKHR>
{
public:
	Surface(const Instance& instance, const Window& window);
	~Surface();

	const VkSurfaceCapabilitiesKHR GetCapabilities(const PhysicalDevice& device) const;
	const VkSurfaceFormatKHR GetFormat(const PhysicalDevice& device) const;
	const VkPresentModeKHR GetPresentMode(const PhysicalDevice& device, bool vsync) const;
private:
	void CreateSurface(const Window& window);
private:
	const Instance& m_Instance;
};