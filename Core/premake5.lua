project "Core"
	kind "StaticLib"

	targetdir ("%{wks.location}/bin/" .. outputdir .. "/%{prj.name}")
	objdir ("%{wks.location}/bin-int/" .. outputdir .. "/%{prj.name}")

	files
	{
		"src/**.h",
		"src/**.cpp",

		"%{IncludeDir.glm}" .. "**.hpp",
		"%{IncludeDir.glm}" .. "**.inl",
		"%{IncludeDir.volk}" .. "**.h"
	}

	includedirs
	{
		"src",
		
		"%{IncludeDir.glfw}",
		"%{IncludeDir.glm}",
		"%{IncludeDir.stb}",
		"%{IncludeDir.tinyobj}",
		"%{IncludeDir.ImGui}",
		"%{IncludeDir.ImGui}/backends",

		"%{IncludeDir.volk}",
		"%{IncludeDir.Vulkan_Headers}",
		"%{IncludeDir.Vulkan_Utility_Libraries}",
		"%{IncludeDir.SPIRV_Reflect}",
		"%{IncludeDir.glslang}",
	}

	libdirs
	{
		"%{LibDir.glslang}"
	}

	links
	{
		"glfw",

		"ImGui",

		"Vulkan-Utility-Libraries",
		"SPIRV-Reflect",
	}

	defines
	{
		"GLSLANG_VALIDATOR_EXECUTABLE_RELATIVE_PATH=" .."\"" .. "%{BinDir.glslang}" .. "\""
	}

	filter "configurations:Debug"
		runtime "Debug"
		symbols "on"

		links
		{
			-- glslang
			"glslangd",
			"GenericCodeGend",
			"glslang-default-resource-limitsd",
			"MachineIndependentd",
			"OSDependentd",
			"SPIRVd",
			"SPIRV-Toolsd",
			"SPIRV-Tools-optd",
			"SPVRemapperd",
		}

	filter "configurations:Release"
		runtime "Release"
		optimize "on"

		links
		{
			-- glslang
			"glslang",
			"GenericCodeGen",
			"glslang-default-resource-limits",
			"MachineIndependent",
			"OSDependent",
			"SPIRV",
			"SPIRV-Tools",
			"SPIRV-Tools-opt",
			"SPVRemapper",
		}

	filter "system:windows"
		systemversion "latest"

	filter "configurations:Debug"
		symbols "On"

	filter "configurations:Release"
		optimize "On"