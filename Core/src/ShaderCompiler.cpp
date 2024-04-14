#include "ShaderCompiler.h"

#include "Log.h"

#include "Utils.h"
#include "Timer.h"

#include <glslang/SPIRV/GlslangToSpv.h>
#include <glslang/Public/ShaderLang.h>
#include <glslang/Include/ResourceLimits.h>

#include <algorithm>
#include <ranges>
#include <sstream>

static constexpr const char* s_SpvExtention = ".spv";
static constexpr const char* const s_ShaderExtensions[] = { ".vert", ".frag" };

static bool HasVertOrFragExtension(const std::filesystem::path& path)
{
	const auto range = std::ranges::subrange(s_ShaderExtensions, s_ShaderExtensions + std::size(s_ShaderExtensions));

	return std::ranges::find_if(range, [&path](const char* extension) { return path.extension() == extension; }) != range.end();
	//return path.extension() == s_ShaderExtensions[0] || path.extension() == s_ShaderExtensions[1];
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

	bool success = true;
	uint32_t shadersCompiledCount = 0;
	for (const auto& path : paths)
	{
		auto p = path;
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
#ifdef _WIN32
		cmd << " > nul";
#else
		cmd << " > /dev/null";
#endif

		int executed = system(cmd.str().data());

		if (/*executed*/std::filesystem::exists(pathWithSpvExtension))
		{
			LOG("Compiled: %s", pathString.data());
			shadersCompiledCount++;
		}
		else
		{
			LOG("Failed to compile shader %s", pathString.data());
			success |= false;
		}
	}

	LOG("Total shaders: %i. Files to compile %i, successfully %i. Total compilation time: %.2f ms",
		totalShadersCount, paths.size(), shadersCompiledCount, timer.ElapsedMS());

	return success;
}
