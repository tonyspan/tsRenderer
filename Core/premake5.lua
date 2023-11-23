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
	}

	includedirs
	{
		"src",
		
		"%{IncludeDir.SDL}",
		"%{IncludeDir.glm}",
		"%{IncludeDir.stb}",
		"%{IncludeDir.tinyobj}",
		"%{IncludeDir.ImGui}",
		"%{IncludeDir.ImGui}/backends",
		"%{IncludeDir.Vulkan}"
	}

	libdirs
	{
		"%{LibDir.SDL}",
		"%{LibDir.Vulkan}"
	}

	links
	{
		"SDL2",
		"SDL2main",
		"SDL2test",

		"ImGui",

		"vulkan-1"
	}

	filter "system:windows"
		systemversion "latest"

	filter "configurations:Debug"
		symbols "On"

	filter "configurations:Release"
		optimize "On"