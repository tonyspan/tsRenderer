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

-- TODO: Move elsewhere?
local function IsDirectory(strFolderName)
  local fileHandle, strError = io.open(strFolderName.."\\*.*", "r")
  if fileHandle ~= nil then
    io.close(fileHandle)
    return true
  else
    if string.match(strError,"No such file or directory") then
      return false
    else
      return true
    end
  end
end

os.execute("python scripts/SetupVulkan.py")
local VulkanSDK = os.getenv("VULKAN_SDK")
assert(VulkanSDK ~= nill, "VULKAN_SDK must exist")

local VulkanLib = VulkanSDK .. "/Lib"
local VulkanIncludeDir = VulkanSDK .. "/Include"

local SDLVersion = "2.28.4"
os.execute("python scripts/SetupSDL.py " .. "3rdparty/SDL" .. " " .. SDLVersion)

local SDLDir = "SDL/SDL2-" .. SDLVersion .. "/"
assert(IsDirectory(SDLDir) ~= nill, "SDL must exist")

IncludeDir = {}
IncludeDir["SDL"] = thirdparty .. SDLDir .. "include"
IncludeDir["glm"] = thirdparty .. "glm"
IncludeDir["stb"] = thirdparty .. "stb"
IncludeDir["tinyobj"] = thirdparty .. "tinyobjloader"
IncludeDir["ImGui"] = thirdparty .. "imgui"

IncludeDir["Vulkan"] = VulkanIncludeDir

LibDir = {}
LibDir["SDL"] = thirdparty .. SDLDir .. "lib/x64"

LibDir["Vulkan"] = VulkanLib

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