workspace "LightBox"
	architecture "x64"
	configurations { "Debug", "Release", "Dist" }
	startproject "LightBox"

outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

group "Dependencies"
	include "vendor/imgui"
group ""

project "LightBox"
	location "LightBox"
	kind "ConsoleApp"
	language "C++"

	targetdir ("bin/" .. outputdir .. "/%{prj.name}")
	objdir ("bin-int/" .. outputdir .. "/%{prj.name}")

	files
	{
		-- ** means recursively search the folders downwards
		"%{prj.name}/src/**.h",
		"%{prj.name}/src/**.cpp"
	}
	includedirs
	{
		"%{prj.name}/src",
		"C:/VulkanSDK/1.3.250.0/Include",
		"vendor/imgui",
		"vendor/glfw-3.3.8.bin.WIN64/Include",
		"vendor/glm",
		"vendor/stb_image"
	} 
	libdirs
	{
		"C:/VulkanSDK/1.3.250.0/Lib",
		"vendor/glfw-3.3.8.bin.WIN64/lib-vc2022"
	}
	links
	{
		-- REMAINDER ImGui is included as a VS Static Lib project
		"ImGui",
		"opengl32.lib",
		"glfw3.lib",
		"vulkan-1.lib"
	}

	filter "system:windows"
		cppdialect "C++17"
		staticruntime "On"
		systemversion "latest"
		defines { "WL_PLATFORM_WINDOWS" }

   filter "configurations:Debug"
		defines { "WL_DEBUG" }
		staticruntime "off"
		runtime "Debug"
		symbols "On"

   filter "configurations:Release"
		defines { "WL_RELEASE" }
		staticruntime "off"
		runtime "Release"
		optimize "On"
		symbols "On"

   filter "configurations:Dist"
		kind "WindowedApp"
		defines { "WL_DIST" }
		staticruntime "off"
		runtime "Release"
		optimize "On"
		symbols "Off"