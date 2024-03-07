#pragma once

#include "Base.h"

#include "VK.h"

#include "Enums.h"

#include <string>
#include <vector>

// DescriptorSetLayout is analogous to a struct definition
// DescriptorSet is an instance of that struct

//Set = 0 descriptor set containing uniform buffer with global, per - frame or per - view data,
// as well as globally available textures such as shadow map texture array / atlas
//Set = 1 descriptor set containing uniform buffer and texture descriptors for per - material data, such as albedo map, Fresnel coefficients, etc.
//Set = 2 descriptor set containing dynamic uniform buffer with per - draw data, such as world transform array

struct DescriptorSetElement
{
	std::string Name;
	uint32_t Binding = ~0;
	DescriptorType Type = DescriptorType::UNDEFINED;
	StageFlag Stage = StageFlag::UNDEFINED;
};

class DescriptorSetLayout
{
public:
	DescriptorSetLayout() = default;

	DescriptorSetLayout(const std::initializer_list<DescriptorSetElement> elements)
		: m_Elements(elements)
	{
	}

	DescriptorSetLayout(const std::vector<DescriptorSetElement>& elements)
		: m_Elements(elements)
	{
	}

	const DescriptorSetElement& GetElement(uint32_t index) const;
	const std::vector<DescriptorSetElement>& GetElements() const;
	uint32_t GetElementCount() const;
private:
	std::vector<DescriptorSetElement> m_Elements;
};

class Texture;
class Buffer;

class DescriptorSet
{
public:
	static Ref<DescriptorSet> Create(const DescriptorSetLayout& layout);

	DescriptorSet(const DescriptorSetLayout& layout);

	~DescriptorSet();

	void SetBuffer(uint32_t binding, const Buffer& buffer);
	void SetTexture(uint32_t binding, const Texture& texture);

	const VkDescriptorSetLayout GetLayout() const;
	VkDescriptorSet GetDescriptorSet() const;
private:
	void CreateDescriptorSetLayout(const DescriptorSetLayout& layout);
	void CreateDescriptorSet();
private:
	VkDescriptorSetLayout m_DescriptorSetLayout;
	std::vector<VkDescriptorSet> m_DescriptorSets;
	//std::map<uint32_t, std::map<uint32_t, VkDescriptorSet>> m_DescriptorSets;
};