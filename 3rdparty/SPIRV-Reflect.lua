project "SPIRV-Reflect"
    location "SPIRV-Reflect"
    kind "StaticLib"
    language "C"

    targetdir ("%{wks.location}/bin/" .. outputdir .. "/%{prj.name}")
	objdir ("%{wks.location}/bin-int/" .. outputdir .. "/%{prj.name}")

    files
    {
		"SPIRV-Reflect/spirv_reflect.c",
		"SPIRV-Reflect/spirv_reflect.h",
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