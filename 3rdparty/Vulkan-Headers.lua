project "Vulkan-Headers"
    location "Vulkan-Headers"
    kind "None"
    language "C++"

    targetdir ("%{wks.location}/bin/" .. outputdir .. "/%{prj.name}")
	objdir ("%{wks.location}/bin-int/" .. outputdir .. "/%{prj.name}")

    files
    {
       "Vulkan-Headers/include/**.h",
    }

	includedirs
	{
		"Vulkan-Headers/include/vk_video",
		"Vulkan-Headers/include/vulkan",
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