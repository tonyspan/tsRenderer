#pragma once

#include "Base.h"

#include "VK.h"

#include "Enums.h"

#include <filesystem>

class Buffer;

class Shader : public Handle<VkShaderModule>
{
public:
	static Ref<Shader> Create(StageFlag stage, const std::filesystem::path& path);

	Shader(StageFlag stage, const std::filesystem::path& path, const Buffer& buffer);
	~Shader();

	const VkPipelineShaderStageCreateInfo& GetCreateInfoForPipeline() const;
private:
	void CreateShaderModule(StageFlag stage, const std::filesystem::path& path, const Buffer& buffer);
	void PopulatePipelineShaderStageCreateInfo(StageFlag stage);
private:
	VkPipelineShaderStageCreateInfo* m_PipelineShaderStageCreateInfo = nullptr;
};