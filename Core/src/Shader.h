#pragma once

#include "Base.h"

#include "VK.h"

#include "Enums.h"

#include <string_view>
#include <vector>

class Shader : public Handle<VkShaderModule>
{
public:
	static Ref<Shader> Create(StageFlag stage, const std::string_view path);

	Shader(StageFlag stage, const std::string_view path, const std::vector<uint8_t>& code);
	~Shader();

	const VkPipelineShaderStageCreateInfo& GetCreateInfoForPipeline() const;

	static void CompileShaders(const std::string& shaderDirectory);
private:
	void CreateShaderModule(StageFlag stage, const std::string_view path, const std::vector<uint8_t>& code);
	void PopulatePipelineShaderStageCreateInfo(StageFlag stage);
private:
	VkPipelineShaderStageCreateInfo* m_PipelineShaderStageCreateInfo = nullptr;
};