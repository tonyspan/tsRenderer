project "Vulkan-Utility-Libraries"
    location "Vulkan-Utility-Libraries"
    kind "None"
    language "C++"

    targetdir ("%{wks.location}/bin/" .. outputdir .. "/%{prj.name}")
	objdir ("%{wks.location}/bin-int/" .. outputdir .. "/%{prj.name}")

    files
    {
        "Vulkan-Utility-Libraries/**.cpp",
        "Vulkan-Utility-Libraries/include/**.h",
        "Vulkan-Utility-Libraries/include/**.hpp",
    }

    includedirs
    {
    }

	filter "system:windows"
		systemversion "latest"

	filter "system:linux"
		pic "On"
		systemversion "latest"

	filter "configurations:Debug"
		runtime "Debug"
		symbols "on"

	filter "configurations:Release"
		runtime "Release"
		optimize "on"