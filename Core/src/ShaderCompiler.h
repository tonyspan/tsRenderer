#pragma once

#include "Enums.h"

#include <filesystem>
#include <string_view>

class Buffer;

class ShaderCompiler
{
public:
	static bool CompileWithValidator(const std::filesystem::path& file, bool suppressOutput);
	static bool CompileWithValidator(const std::filesystem::path& shaderDirectory);

	static bool Compile(Buffer& buffer, StageFlag stage, const std::string_view code);
};