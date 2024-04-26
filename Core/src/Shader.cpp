#include "Shader.h"

#include "Context.h"
#include "Device.h"

#include "Log.h"

#include "Buffer.h"

#include "Utils.h"

#include <volk.h>
#include <vulkan/vulkan.h>

#include <spirv_reflect.h>

#include <utility>
#include <ranges>
#include <map>

static constexpr const char* s_LogTag = "[Shader]";

#if _DEBUG
#define REFLECTION_DEBUG_LOG(...) LOG_TAGGED(s_LogTag, __VA_ARGS__);
#else
#define REFLECTION_DEBUG_LOG(...) ((void)0)
#endif

static VkDescriptorType Convert(SpvReflectDescriptorType type)
{
#define SPV_REFLECT_DESCRIPTOR_TYPE_CASE(X) \
	case SPV_REFLECT_DESCRIPTOR_TYPE_##X: \
		return VK_DESCRIPTOR_TYPE_##X

	switch (type)
	{
		SPV_REFLECT_DESCRIPTOR_TYPE_CASE(UNIFORM_BUFFER);
		SPV_REFLECT_DESCRIPTOR_TYPE_CASE(COMBINED_IMAGE_SAMPLER);
	default:
		break;
	}

	ASSERT(false);
	return VK_DESCRIPTOR_TYPE_MAX_ENUM;
}

static VkFormat Convert(SpvReflectFormat type)
{
#define SPV_REFLECT_FORMAT_CASE(X) \
	case SPV_REFLECT_FORMAT_##X: \
		return VK_FORMAT_##X

	switch (type)
	{
		SPV_REFLECT_FORMAT_CASE(R32_UINT);
		SPV_REFLECT_FORMAT_CASE(R32_SFLOAT);
		SPV_REFLECT_FORMAT_CASE(R32G32_SFLOAT);
		SPV_REFLECT_FORMAT_CASE(R32G32B32_SFLOAT);
		SPV_REFLECT_FORMAT_CASE(R32G32B32A32_SFLOAT);
	default:
		break;
	}

	ASSERT(false);
	return VK_FORMAT_UNDEFINED;
}

Ref<ShaderModule> ShaderModule::Create(StageFlag stage, const std::filesystem::path& path)
{
	Buffer buffer;

	if (!ReadFromFile(buffer, path))
		return nullptr;

	Ref<ShaderModule> shaderModule = CreateRef<ShaderModule>(stage, path, buffer);

	buffer.Release();

	return shaderModule;
}

Ref<ShaderModule> ShaderModule::Create(StageFlag stage, const Buffer& buffer)
{
	const std::string& stringAsPath = std::string(ShaderStageString(stage)) + " " STR(ShaderModule) " created from " STR(Buffer);

	return CreateRef<ShaderModule>(stage, stringAsPath, buffer);
}

ShaderModule::ShaderModule(StageFlag stage, const std::filesystem::path& path, const Buffer& buffer)
	: m_PipelineShaderStageCreateInfo(new VkPipelineShaderStageCreateInfo())
	, m_Stage(stage), m_Path(path), m_Code(new Buffer())
{
	*m_Code = Buffer::Copy(buffer);
	CreateShaderModule();
}

ShaderModule::~ShaderModule()
{
	delete m_PipelineShaderStageCreateInfo;
	m_PipelineShaderStageCreateInfo = nullptr;

	m_Code->Release();
	delete m_Code;
	m_Code = nullptr;

	vkDestroyShaderModule(Context::GetDevice().GetHandle(), Handle::GetHandle(), nullptr);
}

const VkPipelineShaderStageCreateInfo& ShaderModule::GetCreateInfoForPipeline() const
{
	ASSERT(m_PipelineShaderStageCreateInfo);

	return *m_PipelineShaderStageCreateInfo;
}

const std::filesystem::path& ShaderModule::GetPath() const
{
	return m_Path;
}

const StageFlag ShaderModule::GetStage() const
{
	ASSERT(StageFlag::UNDEFINED != m_Stage);

	return m_Stage;
}

const Buffer& ShaderModule::GetCode() const
{
	ASSERT(m_Code && *m_Code);

	return *m_Code;
}

void ShaderModule::CreateShaderModule()
{
	ASSERT(m_Code && *m_Code);

	const size_t codeSize = static_cast<size_t>(m_Code->GetSize());

	LOG_TAGGED(s_LogTag, "%s: %s, size: %i", ShaderStageString(m_Stage), QUOTED(m_Path.string()), codeSize);

	VkShaderModuleCreateInfo shaderModuleInfo;
	ZeroInitVkStruct(shaderModuleInfo, VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO);

	shaderModuleInfo.codeSize = codeSize;
	shaderModuleInfo.pCode = m_Code->As<const uint32_t*>();

	auto& shaderHandle = Handle::GetHandle();

	VkResult result = vkCreateShaderModule(Context::GetDevice().GetHandle(), &shaderModuleInfo, nullptr, &shaderHandle);
	VK_CHECK_RESULT(result);
	ASSERT(Handle::GetHandle(), "Shader module creation failed");

	PopulatePipelineShaderStageCreateInfo(m_Stage);
}

void ShaderModule::PopulatePipelineShaderStageCreateInfo(StageFlag stage)
{
	ZeroInitVkStruct(*m_PipelineShaderStageCreateInfo, VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO);

	(*m_PipelineShaderStageCreateInfo).stage = Convert(stage);
	(*m_PipelineShaderStageCreateInfo).pName = "main";
	(*m_PipelineShaderStageCreateInfo).module = Handle::GetHandle();
}

Ref<Shader> Shader::Create(const std::vector<std::pair<StageFlag, std::filesystem::path>>& shaderModules)
{
	std::vector<Ref<ShaderModule>> sm(shaderModules.size());
	for (size_t i = 0; const auto & [stage, path] : shaderModules)
		sm[i++] = ShaderModule::Create(stage, path);

	ASSERT(std::ranges::all_of(sm, [](const auto& shaderModule) { return nullptr != shaderModule; }),
		"Creation of one of the shader modules failed");

	return CreateRef<Shader>(std::move(sm));
}

Ref<Shader> Shader::Create(const std::vector<std::pair<StageFlag, Buffer>>& shaderModules)
{
	std::vector<Ref<ShaderModule>> sm(shaderModules.size());
	for (size_t i = 0; const auto & [stage, buffer] : shaderModules)
		sm[i++] = ShaderModule::Create(stage, buffer);

	ASSERT(std::ranges::all_of(sm, [](const auto& shaderModule) { return nullptr != shaderModule; }),
		"Creation of one of the shader modules failed");

	return CreateRef<Shader>(std::move(sm));
}

Shader::Shader(const std::vector<Ref<ShaderModule>>& shaderModules)
	: m_ShaderModules(shaderModules)
{
	ReflectShaders();
	CreateDescriptorSetLayout();
	CreatePushConstantRanges();
}

Shader::~Shader()
{
	for (auto& shader : m_ShaderModules)
		shader.reset();

	for (auto& layout : m_SetLayouts)
		vkDestroyDescriptorSetLayout(Context::GetDevice().GetHandle(), layout, nullptr);
}

const std::vector<VkVertexInputAttributeDescription>& Shader::GetAttributeDescriptions() const
{
	return m_VertexInputAttributeDescriptions;
}

const uint32_t Shader::GetVertexInputStride() const
{
	ASSERT(0 != m_VertexInputStride);

	return m_VertexInputStride;
}

const std::vector<WeakRef<ShaderModule>> Shader::GetShaderModules() const
{
	std::vector<WeakRef<ShaderModule>> result(m_ShaderModules.size());

	std::ranges::transform(m_ShaderModules, result.begin(), [](const auto& ptr) { return ptr; });

	return result;
}

const std::vector<VkDescriptorSetLayout>& Shader::GetLayouts() const
{
	return m_SetLayouts;
}

const std::vector<VkPushConstantRange>& Shader::GetPushConstants() const
{
	return m_Ranges;
}

const ShaderResource* Shader::TryGetResource(const std::string& name) const
{
	ID id = HashString(name);

	if (m_ResourcesMap.contains(id))
		return &m_ResourcesMap.at(id);

	LOG_TAGGED(s_LogTag, "Resource %s not found", QUOTED(name));

	return nullptr;
}

const ShaderPushConstant* Shader::TryGetPushConstant(const std::string& name) const
{
	ID id = HashString(name);

	if (m_PushConstantsMap.contains(id))
		return &m_PushConstantsMap.at(id);

	LOG_TAGGED(s_LogTag, "Push constant %s not found", QUOTED(name));

	return nullptr;
}

void Shader::ReflectShaders()
{
	uint32_t offset = 0;
	for (const auto& shaderModule : m_ShaderModules)
	{
		const auto& pathString = shaderModule->GetPath().filename().string();
		const auto& code = shaderModule->GetCode();
		const auto shaderStage = shaderModule->GetStage();

		LOG_TAGGED(s_LogTag, "Reflecting: %s", QUOTED(pathString));

		SpvReflectShaderModule moduleToReflect;

		auto result = spvReflectCreateShaderModule(code.GetSize(), code.As<const uint32_t*>(), &moduleToReflect);
		ASSERT(result == SPV_REFLECT_RESULT_SUCCESS, "Failed to reflect shader %s", QUOTED(pathString));

		uint32_t count = 0;
		if (StageFlag::VERTEX == shaderStage)
		{
			result = spvReflectEnumerateInputVariables(&moduleToReflect, &count, nullptr);
			ASSERT(result == SPV_REFLECT_RESULT_SUCCESS);

			std::vector<SpvReflectInterfaceVariable*> inputVars(count);
			result = spvReflectEnumerateInputVariables(&moduleToReflect, &count, inputVars.data());
			ASSERT(result == SPV_REFLECT_RESULT_SUCCESS);

			//std::reverse(inputVars.begin(), inputVars.end());
			std::ranges::sort(inputVars, [](const auto first, const auto second)
				{
					return first->location < second->location;
				});

			REFLECTION_DEBUG_LOG("Vertex input attributes:");

			m_VertexInputStride = 0;

			for (const auto& inputVar : inputVars)
			{
				VkVertexInputAttributeDescription& attributeDescription = m_VertexInputAttributeDescriptions.emplace_back();

				attributeDescription.location = inputVar->location;
				attributeDescription.binding = 0;
				attributeDescription.format = Convert(inputVar->format);
				attributeDescription.offset = m_VertexInputStride;

				m_VertexInputStride += GetStrideFromFormat(Convert(inputVar->format));

				REFLECTION_DEBUG_LOG("\tlocation = %i %s", attributeDescription.location, QUOTED(inputVar->name));
			}
		}

		count = 0;
		result = spvReflectEnumerateDescriptorSets(&moduleToReflect, &count, nullptr);
		ASSERT(result == SPV_REFLECT_RESULT_SUCCESS);

		std::vector<SpvReflectDescriptorSet*> reflectedDescriptorSets(count);
		spvReflectEnumerateDescriptorSets(&moduleToReflect, &count, reflectedDescriptorSets.data());

		for (const auto& reflectedSet : reflectedDescriptorSets)
		{
			const uint32_t set = reflectedSet->set;

			ShaderResource resource;
			for (uint32_t i = 0; i < reflectedSet->binding_count; i++)
			{
				const SpvReflectDescriptorBinding* reflectedBinding = reflectedSet->bindings[i];
				const auto reflectedBindingName = reflectedBinding->name;

				resource.Name = reflectedBindingName;
				resource.Set = set;
				resource.Binding = reflectedBinding->binding;
				resource.Type = Convert(reflectedBinding->descriptor_type);
				resource.DescriptorCount = reflectedBinding->count;
				resource.Stage = Convert(shaderStage);

				const auto memberCount = reflectedBinding->block.member_count;

				auto& bufferInfos = resource.BufferInfos;
				bufferInfos.resize(memberCount);

				// e.g layout (binding = 0) uniform UBO { mat4 Model; mat4 View; mat4 Projection; } ubo;
				// Get the uniforms 'Model', 'View', 'Projection'
				// and then would be possible to do DescriptorSet->SetUniform("ubo.Model", &model); ?
				for (uint32_t j = 0; j < memberCount; j++)
				{
					const SpvReflectBlockVariable& member = reflectedBinding->block.members[j];

					const std::string& combinedName = std::string(reflectedBindingName) + "." + member.name;

					bufferInfos[j].Name = combinedName;
					bufferInfos[j].Offset = member.offset;
					bufferInfos[j].Range = member.size;
				}

				ID id = HashString(resource.Name);

				// NOTE: Dangerous things can happen
				m_ResourcesMap[id] = resource;
			}

			REFLECTION_DEBUG_LOG("Set #%i:\n\t\tBinding #%i: %s %s",
				set, resource.Binding, QUOTED(DescriptorTypeString(Convert(resource.Type))), QUOTED(resource.Name));

		}

		count = 0;
		result = spvReflectEnumeratePushConstantBlocks(&moduleToReflect, &count, nullptr);
		ASSERT(result == SPV_REFLECT_RESULT_SUCCESS);

		std::vector<SpvReflectBlockVariable*> reflectedPushConstantRanges(count);
		spvReflectEnumeratePushConstantBlocks(&moduleToReflect, &count, reflectedPushConstantRanges.data());

		for (const auto& reflectedPC : reflectedPushConstantRanges)
		{
			const auto reflectedPCName = reflectedPC->name;

			// TODO: Maybe ASSERT?
			if (reflectedPC->size != reflectedPC->padded_size)
				REFLECTION_DEBUG_LOG("Size missmatch for %s", QUOTED(reflectedPCName));

			for (uint32_t i = 0; i < reflectedPC->member_count; i++)
			{
				const auto& member = reflectedPC->members[i];

				// TODO: Maybe ASSERT?
				if (member.size != member.padded_size)
					REFLECTION_DEBUG_LOG("Size missmatch for %s", QUOTED(member.name));

				const std::string& combinedName = std::string(reflectedPCName) + "." + member.name;

				ShaderPushConstant pc;
				pc.Name = combinedName;
				pc.Stage = Convert(shaderStage);
				pc.Size = member.size;
				pc.Offset = member.offset;

				pc.Offset = offset;
				offset += pc.Size;

				ID id = HashString(combinedName);

				if (!m_PushConstantsMap.contains(id))
				{
					m_PushConstantsMap.emplace(id, pc);
				}
				else
				{
					// Not tested
					m_PushConstantsMap.at(id).Stage |= Convert(shaderStage);
				}

				REFLECTION_DEBUG_LOG("Push Constant:\n\t\t%s %s %i %i",
					QUOTED(pc.Name), QUOTED(ShaderStageString(shaderStage)), pc.Size, pc.Offset);
			}
		}

		spvReflectDestroyShaderModule(&moduleToReflect);
	}
}

void Shader::CreateDescriptorSetLayout()
{
	std::vector<VkDescriptorSetLayoutBinding> bindings(m_ResourcesMap.size());
	for (size_t i = 0; const auto & [_, res] : m_ResourcesMap)
	{
		bindings[i].binding = res.Binding;
		bindings[i].descriptorCount = res.DescriptorCount;
		bindings[i].descriptorType = res.Type;
		bindings[i].stageFlags = res.Stage;

		i++;
	}

	VkDescriptorSetLayoutCreateInfo layoutInfo;
	ZeroInitVkStruct(layoutInfo, VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO);

	layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
	layoutInfo.pBindings = bindings.data();

	VkDescriptorSetLayout setLayout = {};
	VkResult result = vkCreateDescriptorSetLayout(Context::GetDevice().GetHandle(), &layoutInfo, nullptr, &setLayout);
	VK_CHECK_RESULT(result);
	ASSERT(setLayout, "Desriptor set layout creation failed");

	m_SetLayouts.emplace_back(setLayout);
}

void Shader::CreatePushConstantRanges()
{
	struct PushConstantRange
	{
		uint32_t MinOffset = ~0;
		uint32_t MaxOffset = 0;
	};

	std::map<VkShaderStageFlags, PushConstantRange> pushConstantRanges;
	for (const auto& [_, pc] : m_PushConstantsMap)
	{
		PushConstantRange& range = pushConstantRanges[pc.Stage];
		range.MinOffset = std::min(range.MinOffset, pc.Offset);
		range.MaxOffset = std::max(range.MaxOffset, pc.Offset + pc.Size);
	}

	for (const auto& [stage, range] : pushConstantRanges)
		m_Ranges.emplace_back(stage, range.MinOffset, range.MaxOffset - range.MinOffset);
}
