#pragma once

#include "Base.h"

#include "VK.h"

#include "Enums.h"

struct SamplerDescription
{
	uint32_t MipLevels = 0;
	Filter MinFilter = Filter::UNDEFINED;
	Filter MagFilter = Filter::UNDEFINED;
};

class Sampler : public Handle<VkSampler>
{
public:
	static Ref<Sampler> Create(const SamplerDescription& desc);

	Sampler(const SamplerDescription& desc);
	~Sampler();
private:
	void CreateSampler();
private:
	SamplerDescription m_Description;
};