project "Sandbox"
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
		"%{IncludeDir.stb}",
		"%{IncludeDir.tinyobj}",
		--"%{IncludeDir.Vulkan}"
	}

	links
	{
		"Core",
	}

	filter "system:windows"
		systemversion "latest"

	filter "configurations:Debug"
		symbols "On"

	filter "configurations:Release"
		optimize "On"
