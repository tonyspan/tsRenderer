#pragma once

#include "VK.h"

class Device;

class DescriptorPool : public Handle<VkDescriptorPool>
{
public:
	DescriptorPool(const Device& device);
	~DescriptorPool();
private:
	void CreateDescriptorPool();
	void Destroy();
private:
	const Device& m_Device;
};