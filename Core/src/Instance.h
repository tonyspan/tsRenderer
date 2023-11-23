#pragma once

#include "Base.h"

#include "VK.h"

class Window;

class Instance : public Handle<VkInstance>
{
public:
	Instance(const Window& window);
	~Instance();
private:
	void CreateInstance(const Window& window);
	void SetupDebugMessenger();
private:
	VkDebugUtilsMessengerEXT m_DebugUtilsMessenger;
};