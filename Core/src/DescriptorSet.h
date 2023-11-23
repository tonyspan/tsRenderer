#pragma once

#include "Base.h"

#include "VK.h"

#include "Enums.h"

#include <vector>
#include <span>

// DescriptorSetLayout is analogous to a struct definition
// DescriptorSet is an instance of that struct

//Set = 0 descriptor set containing uniform buffer with global, per - frame or per - view data,
// as well as globally available textures such as shadow map texture array / atlas
//Set = 1 descriptor set containing uniform buffer and texture descriptors for per - material data, such as albedo map, Fresnel coefficients, etc.
//Set = 2 descriptor set containing dynamic uniform buffer with per - draw data, such as world transform array

#pragma region DescriptorSetLayout

struct DescriptorSetLayoutBinding
{
	uint32_t Binding;
	DescriptorType Type;
	StageFlag StageFlags;

	DescriptorSetLayoutBinding();
	explicit DescriptorSetLayoutBinding(uint32_t binding, DescriptorType type, StageFlag flags);
};

class DescriptorSetLayout : public Handle<VkDescriptorSetLayout>
{
public:
	static Ref<DescriptorSetLayout> Create(const std::vector<DescriptorSetLayoutBinding>& bindings);

	DescriptorSetLayout(const std::vector<DescriptorSetLayoutBinding>& bindings);
	~DescriptorSetLayout();
};

#pragma endregion

#pragma region DescriptorSet

class Texture;
class Buffer;

struct DescriptorSetElement
{
	enum class Type { NONE = 0, UNIFORM, SAMPLER };

	uint32_t Binding;
	Type Tyype;
	uint32_t Size;
	const Texture* Tex;

	DescriptorSetElement();
	explicit DescriptorSetElement(uint32_t binding, DescriptorSetElement::Type type, uint32_t size, const Texture* texture);
};

struct DescriptorSetDescription
{
	const DescriptorSetLayout* DescSetLayout;
	std::span<DescriptorSetElement> Elements;

	DescriptorSetDescription();
};

class DescriptorSet
{
	struct UniformBufferData
	{
		std::vector<Ref<Buffer>> UniformBuffers;
		bool ToFree = false;
	};
public:
	static Ref<DescriptorSet> Create(const DescriptorSetDescription& desc);

	DescriptorSet(const DescriptorSetDescription& desc);
	~DescriptorSet();

	void Update(uint32_t slot, void* data, VkDeviceSize size);

	VkDescriptorSet GetDescriptorSet() const;
private:
	std::vector<UniformBufferData> m_UniformBufferData;
	std::vector<VkDescriptorSet> m_DescriptorSets;
};

#pragma endregion

#include <string>

#include "Enums.h"

struct DescriptorSetElement2
{
	std::string Name;
	uint32_t Binding = ~0;
	DescriptorType Type = DescriptorType::UNDEFINED;
	StageFlag StageFlags = StageFlag::UNDEFINED;

	//DescriptorSetElement2() = default;

	//DescriptorSetElement2(uint32_t binding, const std::string_view name, DescriptorType type, StageFlag stageFlag)
	//	: Binding(binding), Name(name), Type(type), StageFlags(stageFlag)
	//{
	//}
};

class DescriptorSetLayout2
{
public:
	DescriptorSetLayout2() = default;

	DescriptorSetLayout2(const std::initializer_list<DescriptorSetElement2> elements)
		: m_Elements(elements)
	{
	}

	DescriptorSetLayout2(const std::vector<DescriptorSetElement2>& elements)
		: m_Elements(elements)
	{
	}

	const std::vector<DescriptorSetElement2>& GetElements() const { return m_Elements; }
	uint32_t GetElementCount() const { return static_cast<uint32_t>(m_Elements.size()); }
private:
	std::vector<DescriptorSetElement2> m_Elements;
};

class DescriptorSet2
{
public:
	DescriptorSet2(const DescriptorSetLayout2& layout)
	{
		CreateDescriptorSet(layout);
	}

	void SetTexture();
	void SetBuffer();
private:
	void CreateDescriptorSet(const DescriptorSetLayout2& layout);
private:
	std::vector<VkDescriptorSet> m_DescriptorSets;
	//std::map<uint32_t, std::map<uint32_t, VkDescriptorSet>> m_DescriptorSets;
};