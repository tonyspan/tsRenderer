project "tinyobjloader"
    location "tinyobjloader"
    kind "None"
    language "C++"

    targetdir ("%{wks.location}/bin/" .. outputdir .. "/%{prj.name}")
	objdir ("%{wks.location}/bin-int/" .. outputdir .. "/%{prj.name}")

    files
    {
        "tinyobjloader/**.hpp",
        "tinyobjloader/**.h"
    }

    includedirs
    {
        "{%IncludeDir.tinyobj}"
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