project "ImGui"
	location "imgui"
	kind "StaticLib"
	language "C++"

	targetdir ("%{wks.location}/bin/" .. outputdir .. "/%{prj.name}")
	objdir ("%{wks.location}/bin-int/" .. outputdir .. "/%{prj.name}")

	files
	{
		"imgui/imconfig.h",
		"imgui/imgui.cpp",
		"imgui/imgui.h",
		"imgui/imgui_internal.h",
		"imgui/imgui_demo.cpp",
		"imgui/imgui_draw.cpp",
		"imgui/imgui_tables.cpp",
		"imgui/imgui_widgets.cpp",
		"imgui/imstb_rectpack.h",
		"imgui/imstb_textedit.h",
		"imgui/imstb_truetype.h",

		"imgui/backends/imgui_impl_vulkan.h",
		"imgui/backends/imgui_impl_vulkan.cpp",
		"imgui/backends/imgui_impl_sdl2.h",
		"imgui/backends/imgui_impl_sdl2.cpp",
	}

	includedirs
	{
		"%{IncludeDir.SDL}",
		"%{IncludeDir.ImGui}",
		"%{IncludeDir.Vulkan}"
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
