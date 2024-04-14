#pragma once

#include <filesystem>

class ShaderCompiler
{
public:
	static bool CompileWithValidator(const std::filesystem::path& shaderDirectory);
};