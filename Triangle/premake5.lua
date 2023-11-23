project "Triangle"
	kind "ConsoleApp"

	targetdir ("%{wks.location}/bin/" .. outputdir .. "/%{prj.name}")
	objdir ("%{wks.location}/bin-int/" .. outputdir .. "/%{prj.name}")

	files
	{
		"*.h",
		"*.cpp"
	}

	includedirs
	{
		"%{wks.location}/Core/src",
		"%{IncludeDir.glm}",
		"%{IncludeDir.SDL}",
		"%{IncludeDir.stb}",
		"%{IncludeDir.tinyobj}",
		"%{IncludeDir.Vulkan}"
	}

	libdirs
	{
		"%{LibDir.Vulkan}"
	}

	links
	{
		"Core",
		
		"vulkan-1"
	}

	postbuildcommands
	{
		"{COPY} %{LibDir.SDL}/**.dll %{cfg.targetdir}"
	}

	filter "system:windows"
		systemversion "latest"

	filter "configurations:Debug"
		symbols "On"

	filter "configurations:Release"
		optimize "On"
