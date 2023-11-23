workspace "tsVKRenderer"
	architecture "x64"
	language "C++"
	cppdialect "C++20"
	characterset ("MBCS")

	startproject "Sandbox"

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

-- Install (system wide) Vulkan SDK directory
-- TODO: Not system wide?
vulkanDir = os.outputof("python scripts/SetupVulkan.py")
vulkanDir = vulkanDir .. "/"

SDLVersion = "2.28.4"

-- Get SDL2 lib
SDLDir = os.outputof("python scripts/SetupSDL.py " .. "3rdparty/SDL" .. " " .. SDLVersion)
SDLDir = SDLDir .. "/"

IncludeDir = {}
IncludeDir["SDL"] = SDLDir .. "include"
IncludeDir["glm"] = thirdparty .. "glm"
IncludeDir["stb"] = thirdparty .. "stb"
IncludeDir["tinyobj"] = thirdparty .. "tinyobjloader"
IncludeDir["ImGui"] = thirdparty .. "imgui"

IncludeDir["Vulkan"] = vulkanDir .. "Include"

LibDir = {}
LibDir["SDL"] = SDLDir .. "lib/x64"

LibDir["Vulkan"] = vulkanDir .. "Lib"

group "3rdparty"
	include "3rdparty/imgui"
	include "3rdparty/glm"
	include "3rdparty/stb"
	include "3rdparty/tinyobj"
group ""

include "Core"
include "Sandbox"
include "Triangle"
include "Cube"