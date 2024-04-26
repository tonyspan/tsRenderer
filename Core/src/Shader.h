#pragma once

#include "Base.h"

#include "VK.h"

#include "Enums.h"

#include <filesystem>
#include <string>
#include <unordered_map>

class Buffer;

class ShaderModule : public Handle<VkShaderModule>
{
public:
	static Ref<ShaderModule> Create(StageFlag stage, const std::filesystem::path& path);
	static Ref<ShaderModule> Create(StageFlag stage, const Buffer& buffer);

	ShaderModule(StageFlag stage, const std::filesystem::path& path, const Buffer& buffer);
	~ShaderModule();

	const VkPipelineShaderStageCreateInfo& GetCreateInfoForPipeline() const;
	const std::filesystem::path& GetPath() const;
	const StageFlag GetStage() const;
	const Buffer& GetCode() const;
private:
	void CreateShaderModule();
	void PopulatePipelineShaderStageCreateInfo(StageFlag stage);
private:
	std::filesystem::path m_Path;
	StageFlag m_Stage = StageFlag::UNDEFINED;
	Buffer* m_Code = nullptr;

	VkPipelineShaderStageCreateInfo* m_PipelineShaderStageCreateInfo = nullptr;
};

struct ShaderResource
{
	struct DescriptorBufferInfo
	{
		std::string Name;
		VkDeviceSize Offset = ~0;
		VkDeviceSize Range = ~0;
	};

	std::vector<DescriptorBufferInfo> BufferInfos;

	std::string Name;
	uint32_t Set = ~0;
	uint32_t Binding = ~0;
	uint32_t DescriptorCount = ~0;
	VkDescriptorType Type = (VkDescriptorType)VK_MAX_VALUE_ENUM;
	VkShaderStageFlags Stage = (VkShaderStageFlags)VK_MAX_VALUE_ENUM;
};

struct ShaderPushConstant
{
	std::string Name;
	uint32_t Offset = ~0;
	uint32_t Size = ~0;
	VkShaderStageFlags Stage = (VkShaderStageFlags)VK_MAX_VALUE_ENUM;
};

class Shader
{
	using ID = uint64_t;
public:
	static Ref<Shader> Create(const std::vector<std::pair<StageFlag, std::filesystem::path>>& shaderModules);
	static Ref<Shader> Create(const std::vector<std::pair<StageFlag, Buffer>>& shaderModules);

	Shader(const std::vector<Ref<ShaderModule>>& shaderModules);
	~Shader();

	const std::vector<VkVertexInputAttributeDescription>& GetAttributeDescriptions() const;
	const uint32_t GetVertexInputStride() const;

	const std::vector<WeakRef<ShaderModule>> GetShaderModules() const;
	const std::vector<VkDescriptorSetLayout>& GetLayouts() const;
	const std::vector<VkPushConstantRange>& GetPushConstants() const;

	const ShaderResource* TryGetResource(const std::string& name) const;
	const ShaderPushConstant* TryGetPushConstant(const std::string& name) const;
private:
	void ReflectShaders();
	void CreateDescriptorSetLayout();
	void CreatePushConstantRanges();
private:
	std::vector<VkVertexInputAttributeDescription> m_VertexInputAttributeDescriptions;
	uint32_t m_VertexInputStride = 0;

	std::vector<Ref<ShaderModule>> m_ShaderModules;
	std::vector<VkDescriptorSetLayout> m_SetLayouts;
	std::vector<VkPushConstantRange> m_Ranges;

	std::unordered_map<ID, ShaderResource> m_ResourcesMap;
	std::unordered_map<ID, ShaderPushConstant> m_PushConstantsMap;
};
