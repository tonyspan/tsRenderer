#pragma once

#include "VK.h"

class Device;

class CommandPool : public Handle<VkCommandPool>
{
public:
	CommandPool(const Device& device);
	~CommandPool();
private:
	void CreateCommandPool();
private:
	const Device& m_Device;
};