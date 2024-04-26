#pragma once

#include "Base.h"

#include "VK.h"

#include "Enums.h"

#include <string>
#include <vector>

// DescriptorSetLayout is analogous to a struct definition
// DescriptorSet is an instance of that struct

// Set = 0 descriptor set containing uniform buffer with global, per - frame or per - view data,
// as well as globally available textures such as shadow map texture array / atlas
// Set = 1 descriptor set containing uniform buffer and texture descriptors for per - material data, such as albedo map, Fresnel coefficients, etc.
// Set = 2 descriptor set containing dynamic uniform buffer with per - draw data, such as world transform array

class Texture;
class GBuffer;
class Shader;

struct DescriptorSetDescription
{
	WeakRef<Shader> Shader;
};

class DescriptorSet
{
public:
	static Ref<DescriptorSet> Create(const DescriptorSetDescription& desc);

	DescriptorSet(const DescriptorSetDescription& desc);
	~DescriptorSet() = default;

	void SetBuffer(uint32_t binding, const GBuffer& buffer);
	void SetBuffer(const std::string& name, const GBuffer& buffer);

	void SetTexture(uint32_t binding, const Texture& texture);
	void SetTexture(const std::string& name, const Texture& texture);

	VkDescriptorSet GetDescriptorSet() const;
private:
	void CreateDescriptorSet();
private:
	std::vector<VkDescriptorSet> m_DescriptorSets;

	uint32_t m_ImageCount = 0;
	WeakRef<Shader> m_Shader;
};
