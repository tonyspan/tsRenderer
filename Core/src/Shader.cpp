#include "Shader.h"

#include "Context.h"
#include "Device.h"

#include "Log.h"

#include "Utils.h"

#include <vulkan/vulkan.h>

#include <vector>
#include <fstream>
#include <filesystem>
#include <iomanip>
#include <sstream>

static constexpr const char* s_spvExtention = ".spv";
static constexpr const char* const s_ShaderExtensions[] = { ".vert", ".frag" };

static bool HasVertOrFragExtension(const std::string_view path)
{
	return path == s_ShaderExtensions[0] || path == s_ShaderExtensions[1];
}

static std::vector<uint8_t> ReadBytes(const std::string_view path)
{
	std::ifstream stream(path.data(), std::ios::binary | std::ios::ate);

	if (!stream.is_open())
	{
		LOG("Failed to open %s", path.data());
		return {};
	}

	std::streampos end = stream.tellg();
	stream.seekg(0, std::ios::beg);
	size_t size = end - stream.tellg();

	std::vector<uint8_t> buffer(size);
	buffer.shrink_to_fit();

	if (size > 0)
	{
		stream.read(reinterpret_cast<char*>(buffer.data()), size);
		stream.close();
	}

	return buffer;
}

Ref<Shader> Shader::Create(StageFlag stage, const std::string_view path)
{
	const auto& code = ReadBytes(path);
	//ASSERT(!code.empty());

	if (!code.empty())
		return CreateRef<Shader>(stage, path, code);

	return nullptr;
}

Shader::Shader(StageFlag stage, const std::string_view path, const std::vector<uint8_t>& code)
	: m_PipelineShaderStageCreateInfo(new VkPipelineShaderStageCreateInfo())
{
	CreateShaderModule(stage, path, code);
}

Shader::~Shader()
{
	delete m_PipelineShaderStageCreateInfo;

	vkDestroyShaderModule(Context::GetDevice().GetHandle(), Handle::GetHandle(), nullptr);
}

const VkPipelineShaderStageCreateInfo& Shader::GetCreateInfoForPipeline() const
{
	return *m_PipelineShaderStageCreateInfo;
}

void Shader::CompileShaders(const std::string& shaderDirectory)
{
	static std::string vulkanSDK = std::getenv("VULKAN_SDK");

	if (vulkanSDK.empty())
	{
		LOG("Vulkan enviromental variable not found");
		//std::exit(EXIT_FAILURE);
		return;
	}

	if (shaderDirectory.empty() || !std::filesystem::exists(shaderDirectory))
		return;

	std::vector<std::filesystem::path> paths;

	if (/*std::filesystem::exists(shaderDirectory) &&*/ std::filesystem::is_directory(shaderDirectory))
	{
		for (auto const& entry : std::filesystem::directory_iterator(shaderDirectory))
		{
			const auto& extension = entry.path().extension();
			if (std::filesystem::is_regular_file(entry) && extension != s_spvExtention && HasVertOrFragExtension(extension.string()))
				paths.emplace_back(entry.path());
		}
	}

	uint32_t shadersCompiledCount = 0;
	for (const auto& path : paths)
	{
		const std::string& file = path.filename().string();
		std::string outputPath = shaderDirectory.data() + file;
		std::string outputPathWithExtension = outputPath + s_spvExtention;

		NormalizePath(outputPath);
		NormalizePath(outputPathWithExtension);

		if (std::filesystem::exists(outputPathWithExtension))
			continue;

		// The escaped quotes are necessary since there may be spaces somewhere in the path
		std::stringstream cmd;
		cmd << vulkanSDK << "\\Bin\\glslangValidator.exe -o " << std::quoted(outputPathWithExtension) << " -V " << std::quoted(outputPath);

		int executed = system(cmd.str().data());

		if (/*executed*/std::filesystem::exists(outputPathWithExtension))
		{
			LOG("Compiled: %s", file.data());
			shadersCompiledCount++;
		}
		else
		{
			LOG("Failed to compile shader %s", file.data());
		}
	}

	LOG("Compiled %i shaders", shadersCompiledCount);
}

void Shader::CreateShaderModule(StageFlag stage, const std::string_view path, const std::vector<uint8_t>& code)
{
	LOG("%s: %s, size: %i", ShaderStageString(stage), path.data(), code.size());

	VkShaderModuleCreateInfo shaderModuleInfo;
	ZeroInitVkStruct(shaderModuleInfo, VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO);

	shaderModuleInfo.codeSize = static_cast<uint32_t>(code.size());
	shaderModuleInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());

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
