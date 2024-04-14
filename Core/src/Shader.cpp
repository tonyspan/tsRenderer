#include "Shader.h"

#include "Context.h"
#include "Device.h"

#include "Log.h"

#include "Buffer.h"

#include "Utils.h"

#include <volk.h>
#include <vulkan/vulkan.h>

Ref<Shader> Shader::Create(StageFlag stage, const std::filesystem::path& path)
{
	Buffer buffer;

	if (!ReadFromFile(buffer, path))
		return nullptr;

	Ref<Shader> shaderModule = CreateRef<Shader>(stage, path, buffer);

	buffer.Release();

	return shaderModule;
}

Shader::Shader(StageFlag stage, const std::filesystem::path& path, const Buffer& buffer)
	: m_PipelineShaderStageCreateInfo(new VkPipelineShaderStageCreateInfo())
{
	CreateShaderModule(stage, path, buffer);
}

Shader::~Shader()
{
	delete m_PipelineShaderStageCreateInfo;

	vkDestroyShaderModule(Context::GetDevice().GetHandle(), Handle::GetHandle(), nullptr);
}

const VkPipelineShaderStageCreateInfo& Shader::GetCreateInfoForPipeline() const
{
	ASSERT(m_PipelineShaderStageCreateInfo);

	return *m_PipelineShaderStageCreateInfo;
}

void Shader::CreateShaderModule(StageFlag stage, const std::filesystem::path& path, const Buffer& buffer)
{
	ASSERT(buffer);

	const uint32_t codeSize = static_cast<uint32_t>(buffer.GetSize());

	LOG("%s: %s, size: %i", ShaderStageString(stage), path.string().data(), codeSize);

	VkShaderModuleCreateInfo shaderModuleInfo;
	ZeroInitVkStruct(shaderModuleInfo, VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO);

	shaderModuleInfo.codeSize = codeSize;
	shaderModuleInfo.pCode = buffer.As<const uint32_t>();

	VkResult result = vkCreateShaderModule(Context::GetDevice().GetHandle(), &shaderModuleInfo, nullptr, &Handle::GetHandle());
	VK_CHECK_RESULT(result);
	ASSERT(Handle::GetHandle(), "Shader module creation failed");

	PopulatePipelineShaderStageCreateInfo(stage);
}

void Shader::PopulatePipelineShaderStageCreateInfo(StageFlag stage)
{
	ZeroInitVkStruct(*m_PipelineShaderStageCreateInfo, VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO);

	(*m_PipelineShaderStageCreateInfo).stage = Convert(stage);
	(*m_PipelineShaderStageCreateInfo).pName = "main";
	(*m_PipelineShaderStageCreateInfo).module = Handle::GetHandle();
}
