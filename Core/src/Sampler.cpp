#include "Sampler.h"

#include "Context.h"
#include "Device.h"

#include "Log.h"

#include <vulkan/vulkan.h>

Ref<Sampler> Sampler::Create(const SamplerDescription& desc)
{
	ASSERT(desc.MipLevels != 0);

	return CreateRef<Sampler>(desc);
}

Sampler::Sampler(const SamplerDescription& desc)
	: m_Description(desc)
{
	CreateSampler();
}

Sampler::~Sampler()
{
	vkDestroySampler(Context::GetDevice().GetHandle(), Handle::GetHandle(), nullptr);
}

void Sampler::CreateSampler()
{
	const auto& device = Context::GetDevice();
	const auto& vkDevice = device.GetHandle();

	VkSamplerCreateInfo samplerInfo;
	ZeroInitVkStruct(samplerInfo, VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO);

	samplerInfo.minFilter = Convert(m_Description.MinFilter);
	samplerInfo.magFilter = Convert(m_Description.MagFilter);
	samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerInfo.anisotropyEnable = VK_TRUE;
	samplerInfo.maxAnisotropy = device.GetPhysicalDevice().GetProperties().limits.maxSamplerAnisotropy;
	samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
	samplerInfo.unnormalizedCoordinates = VK_FALSE;
	samplerInfo.compareEnable = VK_FALSE;
	samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
	samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
	samplerInfo.mipLodBias = 0.0f;
	samplerInfo.minLod = 0.0f;
	samplerInfo.maxLod = static_cast<float>(m_Description.MipLevels);

	VkResult result = vkCreateSampler(vkDevice, &samplerInfo, nullptr, &Handle::GetHandle());
	VK_CHECK_RESULT(result);
	ASSERT(Handle::GetHandle(), "Sampler creation failed");
}
