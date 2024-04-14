#include "DescriptorSet.h"

#include "Context.h"
#include "Device.h"
#include "Swapchain.h"
#include "DescriptorPool.h"
#include "Image.h"
#include "Texture.h"
#include "GBuffer.h"
#include "Sampler.h"

#include "Log.h"

#include <volk.h>
#include <vulkan/vulkan.h>

const DescriptorSetElement& DescriptorSetLayout::GetElement(uint32_t index) const
{
	const size_t idx = static_cast<size_t>(index);
	ASSERT(idx >= 0 && idx < m_Elements.size());

	return m_Elements[idx];
}

const std::vector<DescriptorSetElement>& DescriptorSetLayout::GetElements() const
{
	return m_Elements;
}

uint32_t DescriptorSetLayout::GetElementCount() const
{
	return static_cast<uint32_t>(m_Elements.size());
}

Ref<DescriptorSet> DescriptorSet::Create(const DescriptorSetLayout& layout)
{
	ASSERT(!layout.GetElements().empty(), "DescriptorSetLayout is empty");

	return CreateRef<DescriptorSet>(layout);
}

DescriptorSet::DescriptorSet(const DescriptorSetLayout& layout)
	: m_DescriptorSetLayout(VK_NULL_HANDLE)
{
	CreateDescriptorSetLayout(layout);
	CreateDescriptorSet();
}

DescriptorSet::~DescriptorSet()
{
	// No need to explicitly clean up descriptor sets, they will be automatically freed when the descriptor pool is destroyed
	// vkFreeDescriptorSets(device, descriptorPool, static_cast<uint32_t>(m_DescriptorSets.size()), m_DescriptorSets.data());

	vkDestroyDescriptorSetLayout(Context::GetDevice().GetHandle(), m_DescriptorSetLayout, nullptr);
}

void DescriptorSet::SetBuffer(uint32_t binding, const GBuffer& buffer)
{
	ASSERT(binding != ~0);

	const uint32_t imageCount = Context::GetSwapchain().GetImageCount();

	VkDescriptorBufferInfo bufferInfo = {};

	VkWriteDescriptorSet descriptorWrite;
	ZeroInitVkStruct(descriptorWrite, VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET);

	for (uint32_t i = 0; i < imageCount; i++)
	{
		bufferInfo.buffer = buffer.GetHandle<VkBuffer>();
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

void DescriptorSet::SetTexture(uint32_t binding, const Texture& texture)
{
	ASSERT(binding != ~0);

	const uint32_t imageCount = Context::GetSwapchain().GetImageCount();

	VkDescriptorImageInfo imageInfo = {};
	imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

	VkWriteDescriptorSet descriptorWrite;
	ZeroInitVkStruct(descriptorWrite, VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET);

	for (uint32_t i = 0; i < imageCount; i++)
	{
		const auto textureType = texture.GetType();
		if (TextureType::TEXTURE2D == textureType)
		{
			const auto& texture2D = static_cast<const Texture2D&>(texture);
			imageInfo.imageView = texture2D.GetImage().GetHandle<VkImageView>();

			auto sampler = texture2D.GetSampler();
			if (sampler)
				imageInfo.sampler = sampler->GetHandle();
		}
		else if (TextureType::CUBE == textureType)
		{
			const auto& textureCube = static_cast<const TextureCube&>(texture);
			imageInfo.imageView = textureCube.GetImage().GetHandle<VkImageView>();

			auto sampler = textureCube.GetSampler();
			if (sampler)
				imageInfo.sampler = sampler->GetHandle();
		}
		else
		{
			ASSERT(false);
		}

		descriptorWrite.dstSet = m_DescriptorSets[i];
		descriptorWrite.dstBinding = binding;
		descriptorWrite.dstArrayElement = 0;

		descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		descriptorWrite.descriptorCount = 1;

		descriptorWrite.pImageInfo = &imageInfo;

		vkUpdateDescriptorSets(Context::GetDevice().GetHandle(), 1, &descriptorWrite, 0, nullptr);
	}
}

const VkDescriptorSetLayout DescriptorSet::GetLayout() const
{
	return m_DescriptorSetLayout;
}

VkDescriptorSet DescriptorSet::GetDescriptorSet() const
{
	uint32_t currentFrame = Context::GetSwapchain().GetCurrentFrame();
	return m_DescriptorSets.at(currentFrame);
}

void DescriptorSet::CreateDescriptorSetLayout(const DescriptorSetLayout& layout)
{
	std::vector<VkDescriptorSetLayoutBinding> bindings(layout.GetElementCount());

	for (uint32_t i = 0; i < layout.GetElementCount(); i++)
	{
		const auto& layoutElement = layout.GetElement(i);

		bindings[i].binding = layoutElement.Binding;
		bindings[i].descriptorType = Convert(layoutElement.Type);
		bindings[i].descriptorCount = 1;
		bindings[i].stageFlags = Convert(layoutElement.Stage);
	}

	VkDescriptorSetLayoutCreateInfo layoutInfo;
	ZeroInitVkStruct(layoutInfo, VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO);

	layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
	layoutInfo.pBindings = bindings.data();

	VkResult result = vkCreateDescriptorSetLayout(Context::GetDevice().GetHandle(), &layoutInfo, nullptr, &m_DescriptorSetLayout);
	VK_CHECK_RESULT(result);
	ASSERT(m_DescriptorSetLayout, "DesriptorSetLayout creation failed");
}

void DescriptorSet::CreateDescriptorSet()
{
	const uint32_t imageCount = Context::GetSwapchain().GetImageCount();

	std::vector<VkDescriptorSetLayout> layouts(imageCount, m_DescriptorSetLayout);

	VkDescriptorSetAllocateInfo allocInfo;
	ZeroInitVkStruct(allocInfo, VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO);

	allocInfo.descriptorPool = Context::GetDescriptorPool().GetHandle();
	allocInfo.descriptorSetCount = static_cast<uint32_t>(layouts.size());
	allocInfo.pSetLayouts = layouts.data();

	m_DescriptorSets.resize(imageCount);

	VkResult result = vkAllocateDescriptorSets(Context::GetDevice().GetHandle(), &allocInfo, m_DescriptorSets.data());
	VK_CHECK_RESULT(result);
}
