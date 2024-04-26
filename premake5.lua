workspace "tsVKRenderer"
	architecture "x64"
	language "C++"
	cppdialect "C++20"
	characterset ("MBCS")
	flags "MultiProcessorCompile"

	startproject "Triangle"

	configurations
	{
		"Debug",
		"Release"
	}

	flags
	{
		"MultiProcessorCompile"
	}

outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"
thirdparty = "%{wks.location}/3rdparty/"

IncludeDir = {}
IncludeDir["glfw"] = thirdparty .. "glfw/include"
IncludeDir["glm"] = thirdparty .. "glm"
IncludeDir["stb"] = thirdparty .. "stb"
IncludeDir["tinyobj"] = thirdparty .. "tinyobjloader"
IncludeDir["ImGui"] = thirdparty .. "imgui"
IncludeDir["volk"] = thirdparty .. "volk"

IncludeDir["Vulkan_Headers"] = thirdparty .. "Vulkan-Headers/include"
IncludeDir["Vulkan_Utility_Libraries"] = thirdparty .. "Vulkan-Utility-Libraries/include"
IncludeDir["SPIRV_Reflect"] = thirdparty .. "SPIRV-Reflect"

os.execute("python scripts/SetupGlslang.py " .. "3rdparty/glslang")

IncludeDir["glslang"] = thirdparty .. "glslang/include"

LibDir = {}
LibDir["glslang"] = thirdparty .. "glslang/lib"

BinDir = {}
BinDir["glslang"] = thirdparty .. "glslang/bin"

group "3rdparty"
	include "3rdparty/glfw"
	include "3rdparty/imgui"
	include "3rdparty/glm"
	include "3rdparty/stb"
	include "3rdparty/tinyobj"
	include "3rdparty/volk"
	include "3rdparty/Vulkan-Headers"
	include "3rdparty/Vulkan-Utility-Libraries"
	include "3rdparty/SPIRV-Reflect"
group ""

include "Core"

group "Examples"
	include "Examples/Sandbox"
	include "Examples/Triangle"
	include "Examples/Cube"
	include "Examples/Wireframe"
group ""