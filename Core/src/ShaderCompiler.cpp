#include "ShaderCompiler.h"

#include "Log.h"

#include "Buffer.h"

#include "Utils.h"
#include "Timer.h"
#include "Thread.h"

#include <glslang/SPIRV/GlslangToSpv.h>
#include <glslang/Public/ShaderLang.h>
#include <glslang/Include/ResourceLimits.h>

#include <algorithm>
#include <ranges>
#include <sstream>

static constexpr const char* s_SpvExtention = ".spv";
static constexpr const char* const s_ShaderExtensions[] = { ".vert", ".frag" };

static constexpr const char* s_LogTag = "[Shader Compiler]";
static constexpr bool s_SuppressOutput = true;

static bool HasVertOrFragExtension(const std::filesystem::path& path)
{
	const auto range = std::ranges::subrange(s_ShaderExtensions, s_ShaderExtensions + std::size(s_ShaderExtensions));

	return std::ranges::find_if(range, [&path](const char* extension) { return path.extension() == extension; }) != range.end();
	//return path.extension() == s_ShaderExtensions[0] || path.extension() == s_ShaderExtensions[1];
}

// Taken from: https://github.com/KhronosGroup/glslang/issues/2207
static TBuiltInResource DefaultResources()
{
	TBuiltInResource Resources;

	Resources.maxLights = 32;
	Resources.maxClipPlanes = 6;
	Resources.maxTextureUnits = 32;
	Resources.maxTextureCoords = 32;
	Resources.maxVertexAttribs = 64;
	Resources.maxVertexUniformComponents = 4096;
	Resources.maxVaryingFloats = 64;
	Resources.maxVertexTextureImageUnits = 32;
	Resources.maxCombinedTextureImageUnits = 80;
	Resources.maxTextureImageUnits = 32;
	Resources.maxFragmentUniformComponents = 4096;
	Resources.maxDrawBuffers = 32;
	Resources.maxVertexUniformVectors = 128;
	Resources.maxVaryingVectors = 8;
	Resources.maxFragmentUniformVectors = 16;
	Resources.maxVertexOutputVectors = 16;
	Resources.maxFragmentInputVectors = 15;
	Resources.minProgramTexelOffset = -8;
	Resources.maxProgramTexelOffset = 7;
	Resources.maxClipDistances = 8;
	Resources.maxComputeWorkGroupCountX = 65535;
	Resources.maxComputeWorkGroupCountY = 65535;
	Resources.maxComputeWorkGroupCountZ = 65535;
	Resources.maxComputeWorkGroupSizeX = 1024;
	Resources.maxComputeWorkGroupSizeY = 1024;
	Resources.maxComputeWorkGroupSizeZ = 64;
	Resources.maxComputeUniformComponents = 1024;
	Resources.maxComputeTextureImageUnits = 16;
	Resources.maxComputeImageUniforms = 8;
	Resources.maxComputeAtomicCounters = 8;
	Resources.maxComputeAtomicCounterBuffers = 1;
	Resources.maxVaryingComponents = 60;
	Resources.maxVertexOutputComponents = 64;
	Resources.maxGeometryInputComponents = 64;
	Resources.maxGeometryOutputComponents = 128;
	Resources.maxFragmentInputComponents = 128;
	Resources.maxImageUnits = 8;
	Resources.maxCombinedImageUnitsAndFragmentOutputs = 8;
	Resources.maxCombinedShaderOutputResources = 8;
	Resources.maxImageSamples = 0;
	Resources.maxVertexImageUniforms = 0;
	Resources.maxTessControlImageUniforms = 0;
	Resources.maxTessEvaluationImageUniforms = 0;
	Resources.maxGeometryImageUniforms = 0;
	Resources.maxFragmentImageUniforms = 8;
	Resources.maxCombinedImageUniforms = 8;
	Resources.maxGeometryTextureImageUnits = 16;
	Resources.maxGeometryOutputVertices = 256;
	Resources.maxGeometryTotalOutputComponents = 1024;
	Resources.maxGeometryUniformComponents = 1024;
	Resources.maxGeometryVaryingComponents = 64;
	Resources.maxTessControlInputComponents = 128;
	Resources.maxTessControlOutputComponents = 128;
	Resources.maxTessControlTextureImageUnits = 16;
	Resources.maxTessControlUniformComponents = 1024;
	Resources.maxTessControlTotalOutputComponents = 4096;
	Resources.maxTessEvaluationInputComponents = 128;
	Resources.maxTessEvaluationOutputComponents = 128;
	Resources.maxTessEvaluationTextureImageUnits = 16;
	Resources.maxTessEvaluationUniformComponents = 1024;
	Resources.maxTessPatchComponents = 120;
	Resources.maxPatchVertices = 32;
	Resources.maxTessGenLevel = 64;
	Resources.maxViewports = 16;
	Resources.maxVertexAtomicCounters = 0;
	Resources.maxTessControlAtomicCounters = 0;
	Resources.maxTessEvaluationAtomicCounters = 0;
	Resources.maxGeometryAtomicCounters = 0;
	Resources.maxFragmentAtomicCounters = 8;
	Resources.maxCombinedAtomicCounters = 8;
	Resources.maxAtomicCounterBindings = 1;
	Resources.maxVertexAtomicCounterBuffers = 0;
	Resources.maxTessControlAtomicCounterBuffers = 0;
	Resources.maxTessEvaluationAtomicCounterBuffers = 0;
	Resources.maxGeometryAtomicCounterBuffers = 0;
	Resources.maxFragmentAtomicCounterBuffers = 1;
	Resources.maxCombinedAtomicCounterBuffers = 1;
	Resources.maxAtomicCounterBufferSize = 16384;
	Resources.maxTransformFeedbackBuffers = 4;
	Resources.maxTransformFeedbackInterleavedComponents = 64;
	Resources.maxCullDistances = 8;
	Resources.maxCombinedClipAndCullDistances = 8;
	Resources.maxSamples = 4;
	Resources.maxMeshOutputVerticesNV = 256;
	Resources.maxMeshOutputPrimitivesNV = 512;
	Resources.maxMeshWorkGroupSizeX_NV = 32;
	Resources.maxMeshWorkGroupSizeY_NV = 1;
	Resources.maxMeshWorkGroupSizeZ_NV = 1;
	Resources.maxTaskWorkGroupSizeX_NV = 32;
	Resources.maxTaskWorkGroupSizeY_NV = 1;
	Resources.maxTaskWorkGroupSizeZ_NV = 1;
	Resources.maxMeshViewCountNV = 4;

	Resources.limits.nonInductiveForLoops = 1;
	Resources.limits.whileLoops = 1;
	Resources.limits.doWhileLoops = 1;
	Resources.limits.generalUniformIndexing = 1;
	Resources.limits.generalAttributeMatrixVectorIndexing = 1;
	Resources.limits.generalVaryingIndexing = 1;
	Resources.limits.generalSamplerIndexing = 1;
	Resources.limits.generalVariableIndexing = 1;
	Resources.limits.generalConstantMatrixVectorIndexing = 1;

	return Resources;
}

bool ShaderCompiler::CompileWithValidator(const std::filesystem::path& file, bool suppressOutput)
{
	if (file.empty() || !std::filesystem::exists(file) || std::filesystem::is_directory(file))
		return false;

	auto p = file;
	const auto& pathString = p.string();
	std::filesystem::path pathWithSpvExtension = pathString + s_SpvExtention;

	NormalizePath(p);
	NormalizePath(pathWithSpvExtension);

	std::stringstream cmd;

	// TODO: Workaround, find a better way dealing with this
	std::filesystem::path validatorPath("../");
	validatorPath /= GLSLANG_VALIDATOR_EXECUTABLE_RELATIVE_PATH;

	NormalizePath(validatorPath);

	cmd << validatorPath.string();

#ifdef _WIN32
	cmd << "\\glslangValidator.exe -o ";
#else
	cmd << "/glslangValidator -o ";
#endif
	cmd << std::quoted(pathWithSpvExtension.string()) << " -V " << std::quoted(p.string());

	// Suppress system() output
	if (suppressOutput)
	{
#ifdef _WIN32
		cmd << " > nul";
#else
		cmd << " > /dev/null";
#endif
	}

	int executed = system(cmd.str().data());

	return (0 < executed) ? true : false;
}

bool ShaderCompiler::CompileWithValidator(const std::filesystem::path& shaderDirectory)
{
	if (shaderDirectory.empty() || !std::filesystem::exists(shaderDirectory) || !std::filesystem::is_directory(shaderDirectory))
		return false;

	uint32_t totalShadersCount = 0;
	std::vector<std::filesystem::path> paths;
	for (const auto& entry : std::filesystem::directory_iterator(shaderDirectory))
	{
		if (entry.is_regular_file())
		{
			std::filesystem::path p;
			if (HasVertOrFragExtension(entry.path()))
			{
				totalShadersCount++;

				p = entry.path();
				p += s_SpvExtention;
			}

			if (!p.empty() && !std::filesystem::exists(p))
				paths.emplace_back(entry.path());
		}
	}

	Timer timer;

	std::vector<Thread> threads(paths.size());

	for (size_t i = 0; const auto & path : paths)
	{
#define WITH_MT 1
#if WITH_MT
		auto& thread = threads[i++];

		thread.Start();

		thread.Submit([&path]()
			{
				CompileWithValidator(path, s_SuppressOutput);
			});
#else
		CompileWithValidator(path, s_SuppressOutput);
#endif
	}

	std::ranges::for_each(threads, [](auto& thread) { thread.Stop(); });

	bool success = true;
	uint32_t shadersCompiledCount = 0;
	for (const auto& path : paths)
	{
		const auto& pathString = path.string();
		std::filesystem::path pathWithSpvExtension = pathString + s_SpvExtention;
		NormalizePath(pathWithSpvExtension);

		if (/*executed*/std::filesystem::exists(pathWithSpvExtension))
		{
			LOG_TAGGED(s_LogTag, "Compiled: %s", QUOTED(pathString));
			shadersCompiledCount++;
		}
		else
		{
			LOG_TAGGED(s_LogTag, "Failed to compile shader %s", QUOTED(pathString));
			success &= false;
		}
	}

	LOG_TAGGED(s_LogTag, "Total shaders: %i. Files to compile %i, successfully %i. Total compilation time: %.2f ms",
		totalShadersCount, paths.size(), shadersCompiledCount, timer.ElapsedMS());

	return success;
}

bool ShaderCompiler::Compile(Buffer& buffer, StageFlag stage, const std::string_view code)
{
	glslang::InitializeProcess();

	Timer timer;

	bool success = true;

	auto Convert = [](StageFlag stage)
		{
			switch (stage)
			{
			case StageFlag::VERTEX:
				return EShLanguage::EShLangVertex;
			case StageFlag::FRAGMENT:
				return EShLanguage::EShLangFragment;
			default:
				break;
			}

			ASSERT(false, "Unknown shader stage");
			return EShLanguage::EShLangCount;
		};

	const auto shaderStage = Convert(stage);
	const auto targetVulkanVersion = glslang::EShTargetVulkan_1_2;

	glslang::TShader shader(shaderStage);

	shader.setEnvInput(glslang::EShSourceGlsl, shaderStage, glslang::EShClientVulkan, targetVulkanVersion);
	shader.setEnvClient(glslang::EShClientVulkan, targetVulkanVersion);
	shader.setEnvTarget(glslang::EshTargetSpv, glslang::EShTargetSpv_1_5);

	const char* ptr = code.data();
	shader.setStrings(&ptr, 1);

	TBuiltInResource defaultResource = {};

	// https://stackoverflow.com/questions/64549729/getting-location-too-large-for-fragment-output-error-while-using-glslang
	defaultResource.maxDrawBuffers = true;

	EShMessages messages = EShMessages::EShMsgDefault;
	bool result = shader.parse(&defaultResource, 450, false, messages);
	if (!result)
	{
		LOG_TAGGED(s_LogTag, "Parse failed: %s", QUOTED(shader.getInfoLog()));
		success &= false;
	}

	glslang::TProgram program;
	program.addShader(&shader);

	messages = EShMessages(EShMessages::EShMsgSpvRules | EShMessages::EShMsgVulkanRules);
	result = program.link(messages);
	if (!result)
	{
		LOG_TAGGED(s_LogTag, "Link failed: %s", QUOTED(program.getInfoLog()));
		success &= false;
	}

	glslang::SpvOptions options = { .validate = true };
	spv::SpvBuildLogger logger;

	std::vector<uint32_t> spirv;
	glslang::GlslangToSpv(*program.getIntermediate(shader.getStage()), spirv, &logger, &options);

	if (!logger.getAllMessages().empty())
	{
		LOG_TAGGED(s_LogTag, "%s", QUOTED(logger.getAllMessages()));
	}

	LOG_TAGGED(s_LogTag, "Total compilation time: %.2f ms", timer.ElapsedMS());

	if (spirv.empty())
	{
		LOG_TAGGED(s_LogTag, "Failed to generate SPIR-V");
		success &= false;
	}
	else
	{
		buffer = Buffer::Copy(spirv.data(), spirv.size() * sizeof(spirv[0]) /* or sizeof(uint32_t) */);
	}

	glslang::FinalizeProcess();

	return success;
}
