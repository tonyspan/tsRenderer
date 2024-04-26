#include "DescriptorSet.h"

#include "Context.h"
#include "Device.h"
#include "Swapchain.h"
#include "DescriptorPool.h"
#include "Image.h"
#include "Texture.h"
#include "GBuffer.h"
#include "Sampler.h"
#include "Shader.h"

#include "Log.h"

#include <volk.h>
#include <vulkan/vulkan.h>

Ref<DescriptorSet> DescriptorSet::Create(const DescriptorSetDescription& desc)
{
	return CreateRef<DescriptorSet>(desc);
}

DescriptorSet::DescriptorSet(const DescriptorSetDescription& desc)
{
	m_ImageCount = Context::GetSwapchain().GetImageCount();
	m_Shader = desc.Shader;

	CreateDescriptorSet();
}

void DescriptorSet::SetBuffer(uint32_t binding, const GBuffer& buffer)
{
	ASSERT(binding != ~0);

	auto& bufferHandle = buffer.GetHandle<VkBuffer>();
	ASSERT(bufferHandle);

	VkDescriptorBufferInfo bufferInfo = {};

	VkWriteDescriptorSet descriptorWrite;
	ZeroInitVkStruct(descriptorWrite, VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET);

	for (uint32_t i = 0; i < m_ImageCount; i++)
	{
		bufferInfo.buffer = bufferHandle;
		bufferInfo.offset = 0;
		bufferInfo.range = VK_WHOLE_SIZE;

		descriptorWrite.dstSet = m_DescriptorSets[i];
		descriptorWrite.dstBinding = binding;
		descriptorWrite.dstArrayElement = 0;

		descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		descriptorWrite.descriptorCount = 1;

		descriptorWrite.pBufferInfo = &bufferInfo;

		vkUpdateDescriptorSets(Context::GetDevice().GetHandle(), 1, &descriptorWrite, 0, nullptr);
	}
}

void DescriptorSet::SetBuffer(const std::string& name, const GBuffer& buffer)
{
	auto shader = m_Shader.lock();
	ASSERT(shader);

	const auto resource = shader->TryGetResource(name);

	if (resource)
		SetBuffer(resource->Binding, buffer);
}

void DescriptorSet::SetTexture(uint32_t binding, const Texture& texture)
{
	ASSERT(binding != ~0);

	VkDescriptorImageInfo imageInfo = {};
	imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

	VkWriteDescriptorSet descriptorWrite;
	ZeroInitVkStruct(descriptorWrite, VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET);

	for (uint32_t i = 0; i < m_ImageCount; i++)
	{
		//const auto textureType = texture.GetType();
		imageInfo.imageView = texture.GetImage().GetHandle<VkImageView>();

		auto sampler = texture.GetSampler();
		if (sampler)
			imageInfo.sampler = sampler->GetHandle();

		descriptorWrite.dstSet = m_DescriptorSets[i];
		descriptorWrite.dstBinding = binding;
		descriptorWrite.dstArrayElement = 0;

		descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		descriptorWrite.descriptorCount = 1;

		descriptorWrite.pImageInfo = &imageInfo;

		vkUpdateDescriptorSets(Context::GetDevice().GetHandle(), 1, &descriptorWrite, 0, nullptr);
	}
}

void DescriptorSet::SetTexture(const std::string& name, const Texture& texture)
{
	auto shader = m_Shader.lock();
	ASSERT(shader);

	const auto resource = shader->TryGetResource(name);

	if (resource)
		SetTexture(resource->Binding, texture);
}

VkDescriptorSet DescriptorSet::GetDescriptorSet() const
{
	if (m_DescriptorSets.empty())
		return VK_NULL_HANDLE;

	uint32_t currentFrame = Context::GetSwapchain().GetCurrentFrame();
	return m_DescriptorSets.at(currentFrame);
}

void DescriptorSet::CreateDescriptorSet()
{
	ASSERT(0 < m_ImageCount);

	auto shader = m_Shader.lock();
	ASSERT(shader);

	const auto& device = Context::GetDevice();

	const auto& layouts = shader->GetLayouts();

	ASSERT(!layouts.empty());

	m_DescriptorSets.resize(m_ImageCount);

	VkDescriptorSetAllocateInfo allocInfo;
	ZeroInitVkStruct(allocInfo, VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO);

	allocInfo.descriptorPool = device.GetDescriptorPool().GetHandle();
	allocInfo.descriptorSetCount = static_cast<uint32_t>(layouts.size());
	allocInfo.pSetLayouts = layouts.data();

	for (auto& set : m_DescriptorSets)
	{
		VkResult result = vkAllocateDescriptorSets(device.GetHandle(), &allocInfo, &set);
		VK_CHECK_RESULT(result);
	}
}
