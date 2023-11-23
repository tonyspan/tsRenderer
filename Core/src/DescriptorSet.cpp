#include "DescriptorSet.h"

#include "Context.h"
#include "Device.h"
#include "Swapchain.h"
#include "DescriptorPool.h"
#include "Image.h"
#include "Texture.h"
#include "Buffer.h"
#include "Sampler.h"

#include "Log.h"

#include <vulkan/vulkan.h>

#pragma region DescriptorSetLayout

DescriptorSetLayoutBinding::DescriptorSetLayoutBinding()
	: Binding(0), Type(DescriptorType::UNDEFINED), StageFlags(StageFlag::UNDEFINED)
{
}

DescriptorSetLayoutBinding::DescriptorSetLayoutBinding(uint32_t binding, DescriptorType type, StageFlag flags)
	: Binding(binding), Type(type), StageFlags(flags)
{
}

Ref<DescriptorSetLayout> DescriptorSetLayout::Create(const std::vector<DescriptorSetLayoutBinding>& bindings)
{
	return CreateRef<DescriptorSetLayout>(bindings);
}

DescriptorSetLayout::DescriptorSetLayout(const std::vector<DescriptorSetLayoutBinding>& bindings)
{
	std::vector<VkDescriptorSetLayoutBinding> vkBindings(bindings.size());

	for (uint32_t i = 0; i < bindings.size(); i++)
	{
		auto& binding = bindings[i];

		vkBindings[i].binding = binding.Binding;
		vkBindings[i].descriptorType = Convert(binding.Type);
		vkBindings[i].descriptorCount = 1;
		vkBindings[i].stageFlags = Convert(binding.StageFlags);
	}

	VkDescriptorSetLayoutCreateInfo layoutInfo;
	ZeroInitVkStruct(layoutInfo, VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO);

	layoutInfo.bindingCount = static_cast<uint32_t>(vkBindings.size());
	layoutInfo.pBindings = vkBindings.data();

	VkResult result = vkCreateDescriptorSetLayout(Context::GetDevice().GetHandle(), &layoutInfo, nullptr, &Handle::GetHandle());
	VK_CHECK_RESULT(result);
	ASSERT(Handle::GetHandle(), "DescriptorSetLayout creation failed");
}

DescriptorSetLayout::~DescriptorSetLayout()
{
	vkDestroyDescriptorSetLayout(Context::GetDevice().GetHandle(), Handle::GetHandle(), nullptr);
}

#pragma endregion

#pragma region DescriptorSet

DescriptorSetElement::DescriptorSetElement()
	: Binding(0), Tyype(DescriptorSetElement::Type::NONE)
	, Size(0), Tex(nullptr)
{
}

DescriptorSetElement::DescriptorSetElement(uint32_t binding, DescriptorSetElement::Type type, uint32_t size, const Texture* texture)
	: Binding(binding), Tyype(type), Size(size), Tex(texture)
{
}

DescriptorSetDescription::DescriptorSetDescription()
	: DescSetLayout(nullptr)
{
}

Ref<DescriptorSet> DescriptorSet::Create(const DescriptorSetDescription& desc)
{
	return CreateRef<DescriptorSet>(desc);
}

DescriptorSet::DescriptorSet(const DescriptorSetDescription& desc)
{
	const uint32_t imageCount = Context::GetSwapchain().GetImageCount();
	const auto& device = Context::GetDevice().GetHandle();
	const auto& descriptorPool = Context::GetDescriptorPool();

	const auto& elements = desc.Elements;
	const auto& layout = desc.DescSetLayout;

	m_UniformBufferData.resize(elements.size());
	m_DescriptorSets.resize(imageCount);

	for (uint32_t i = 0; const auto & element : elements)
	{
		auto& ubData = m_UniformBufferData.at(i);

		ubData.UniformBuffers.resize(imageCount);

		if (element.Tyype == DescriptorSetElement::Type::UNIFORM)
		{
			for (uint32_t j = 0; j < imageCount; j++)
				ubData.UniformBuffers[j] = Buffer::CreateUniform(element.Size);

			ubData.ToFree = true;
		}

		i++;
	}

	std::vector<VkDescriptorSetLayout> layouts(imageCount, layout->GetHandle());

	VkDescriptorSetAllocateInfo allocInfo;
	ZeroInitVkStruct(allocInfo, VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO);

	allocInfo.descriptorPool = descriptorPool.GetHandle();
	allocInfo.descriptorSetCount = static_cast<uint32_t>(layouts.size());
	allocInfo.pSetLayouts = layouts.data();

	VkResult result = vkAllocateDescriptorSets(device, &allocInfo, m_DescriptorSets.data());
	VK_CHECK_RESULT(result);

	struct DescriptorSetData
	{
		VkWriteDescriptorSet DescriptorWrite;
		VkDescriptorBufferInfo BufferInfo;
		VkDescriptorImageInfo ImageInfo;
	};

	std::vector<DescriptorSetData> descriptorData(elements.size());
	for (uint32_t i = 0; i < imageCount; i++)
	{
		for (size_t j = 0; j < elements.size(); j++)
		{
			auto& element = elements[j];

			auto& descriptorWrite = descriptorData[j].DescriptorWrite;
			auto& bufferInfo = descriptorData[j].BufferInfo;
			auto& imageInfo = descriptorData[j].ImageInfo;

			ZeroInitVkStruct(descriptorWrite, VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET);

			descriptorWrite.dstSet = m_DescriptorSets[i];
			descriptorWrite.dstBinding = element.Binding;
			descriptorWrite.dstArrayElement = 0;
			descriptorWrite.descriptorCount = 1;

			switch (element.Tyype)
			{
			case DescriptorSetElement::Type::UNIFORM:
			{
				bufferInfo.buffer = m_UniformBufferData.at(j).UniformBuffers.at(i)->GetHandle();
				bufferInfo.offset = 0;
				bufferInfo.range = element.Size;

				descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
				descriptorWrite.pBufferInfo = &bufferInfo;

				break;
			}
			case DescriptorSetElement::Type::SAMPLER:
			{
				imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

				if (element.Tex->GetType() == TextureType::TEXTURE)
				{
					const Texture2D* texture = (const Texture2D*)element.Tex;
					imageInfo.imageView = texture->GetImage().GetImageViewHandle();
					imageInfo.sampler = texture->GetSampler().GetHandle();
				}
				else
				{
					const TextureCube* texture = (const TextureCube*)element.Tex;
					imageInfo.imageView = texture->GetImage().GetImageViewHandle();
					imageInfo.sampler = texture->GetSampler().GetHandle();
				}

				descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
				descriptorWrite.pImageInfo = &imageInfo;

				break;
			}
			default:
			{
				ASSERT(false, "Unknown DescriptorSetElement");
				break;
			}
			}
		}

		for (size_t i = 0; i < descriptorData.size(); i++)
			vkUpdateDescriptorSets(device, 1, &descriptorData.at(i).DescriptorWrite, 0, nullptr);
	}
}

DescriptorSet::~DescriptorSet()
{
	for (auto& ubData : m_UniformBufferData)
	{
		if (ubData.ToFree)
		{
			for (auto& buffer : ubData.UniformBuffers)
				buffer.reset();
		}
		else
		{
			LOG("Not Freed");
		}
	}
}

void DescriptorSet::Update(uint32_t slot, void* data, VkDeviceSize size)
{
	const uint32_t currentImage = Context::GetSwapchain().GetCurrentImage();

	ASSERT(!m_DescriptorSets.empty() || !m_UniformBufferData.at(0).UniformBuffers.empty());
	ASSERT(slot < m_UniformBufferData.size() && currentImage < m_UniformBufferData.at(slot).UniformBuffers.size(), "Index out of bounds");

	m_UniformBufferData.at(slot).UniformBuffers.at(currentImage)->SetData(data, size);
}

VkDescriptorSet DescriptorSet::GetDescriptorSet() const
{
	uint32_t currentFrame = Context::GetSwapchain().GetCurrentFrame();
	return m_DescriptorSets.at(currentFrame);
}

#pragma endregion

void DescriptorSet2::CreateDescriptorSet(const DescriptorSetLayout2& layout)
{
	std::vector<VkDescriptorSetLayoutBinding> vkBindings(layout.GetElementCount());

	for (uint32_t i = 0; i < layout.GetElementCount(); i++)
	{
		auto& binding = layout.GetElements()[i];

		vkBindings[i].binding = binding.Binding;
		vkBindings[i].descriptorType = Convert(binding.Type);
		vkBindings[i].descriptorCount = 1;
		vkBindings[i].stageFlags = Convert(binding.StageFlags);
	}

	VkDescriptorSetLayoutCreateInfo layoutInfo;
	ZeroInitVkStruct(layoutInfo, VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO);

	layoutInfo.bindingCount = static_cast<uint32_t>(vkBindings.size());
	layoutInfo.pBindings = vkBindings.data();

	VkDescriptorSetLayout vkLayout;

	VkResult result = vkCreateDescriptorSetLayout(Context::GetDevice().GetHandle(), &layoutInfo, nullptr, &vkLayout);
	VK_CHECK_RESULT(result);
	ASSERT(vkLayout, "DescriptorSetLayout creation failed");

	VkDescriptorSetAllocateInfo allocInfo;
	ZeroInitVkStruct(allocInfo, VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO);

	allocInfo.descriptorPool = Context::GetDescriptorPool().GetHandle();
	allocInfo.descriptorSetCount = 1;
	allocInfo.pSetLayouts = &vkLayout;

	result = vkAllocateDescriptorSets(Context::GetDevice().GetHandle(), &allocInfo, m_DescriptorSets.data());
	VK_CHECK_RESULT(result);
}
