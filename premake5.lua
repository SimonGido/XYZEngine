workspace "XYZEngine"
		architecture "x64"
		startproject "XYZEditor"

		configurations
		{
			"Debug",
			"Release"
		}

		flags
		{
			"MultiProcessorCompile"
		}
		
outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

IncludeDir = {}
IncludeDir["GLFW"] = "XYZEngine/vendor/GLFW/include"
IncludeDir["GLEW"] = "XYZEngine/vendor/GLEW/include"
IncludeDir["glm"] = "XYZEngine/vendor/glm"
IncludeDir["RCC"] = "XYZEngine/vendor/RCC"
IncludeDir["mini"] = "XYZEngine/vendor/mini"
IncludeDir["OpenAL"] = "XYZEngine/vendor/OpenAL-Soft"
IncludeDir["MiniMp3"] = "XYZEngine/vendor/minimp3"
IncludeDir["FreeType"] = "XYZEngine/vendor/freetype-2.10.1"
IncludeDir["FileWatcher"] = "XYZEngine/vendor/FileWatcher/include"

include "XYZEngine/vendor/GLFW"
include "XYZEngine/vendor/GLEW"
include "XYZEngine/vendor/OpenAL-Soft"
include "XYZEngine/vendor/freetype-2.10.1"

project "XYZEngine"
		location "XYZEngine"
		kind "StaticLib"
		language "C++"
		cppdialect "C++17"
		staticruntime "on"

		targetdir("bin/" .. outputdir .. "/%{prj.name}")
		objdir ("bin-int/" .. outputdir .. "/%{prj.name}")

		pchheader "stdafx.h"
		pchsource "XYZEngine/src/stdafx.cpp"

		files
		{
			"%{prj.name}/src/**.h",
			"%{prj.name}/src/**.cpp",
			"%{prj.name}/src/**.def",
			"%{prj.name}/vendor/glm/glm/**.hpp",
			"%{prj.name}/vendor/glm/glm/**.inl",
			"%{prj.name}/vendor/mini/ini.h",
			"%{prj.name}/vendor/stb_image/**.h",
			"%{prj.name}/vendor/stb_image/**.cpp",

			"%{prj.name}/vendor/yaml-cpp/src/**.cpp",
			"%{prj.name}/vendor/yaml-cpp/src/**.h",
			"%{prj.name}/vendor/yaml-cpp/include/**.h",

			"%{prj.name}/vendor/RCC/**.h",
			"%{prj.name}/vendor/RCC/**.cpp"
		}

		defines
		{
			"_CRT_SECURE_NO_WARNINGS",
			"GLFW_INCLUDE_NONE",
			"GLEW_STATIC",
			"AL_LIBTYPE_STATIC"	
		}

		includedirs
		{
			"%{prj.name}/src",
			"%{IncludeDir.GLFW}",
			"%{IncludeDir.GLEW}",
			"%{IncludeDir.glm}",
			"%{IncludeDir.RCC}",
			"%{IncludeDir.mini}",
			"%{IncludeDir.OpenAL}/include",
			"%{IncludeDir.OpenAL}/src",
			"%{IncludeDir.OpenAL}/src/common",
			"%{IncludeDir.MiniMp3}",	
			"%{prj.name}/vendor/stb_image",
			"%{prj.name}/vendor/yaml-cpp/include",
			"%{IncludeDir.FreeType}/include"
		}

		links
		{
			"GLEW",
			"GLFW",
			"OpenAL-Soft",
			"FreeType",
			"opengl32"
		}
		
		flags { "NoPCH" }
		
		filter "system:windows"
				systemversion "latest"


		filter "configurations:Debug"
				defines "XYZ_DEBUG"
				runtime "Debug"
				symbols "on"


		filter "configurations:Release"
				defines "XYZ_RELEASE"
				runtime "Release"
				optimize "on"

	
project "XYZEditor"
		location "XYZEditor"
		kind "ConsoleApp"
		language "C++"
		cppdialect "C++17"
		staticruntime "on"

		targetdir ("bin/" .. outputdir .. "/%{prj.name}")
		objdir ("bin-int/" .. outputdir .. "/%{prj.name}")

		files
		{
			"%{prj.name}/src/**.h",
			"%{prj.name}/src/**.cpp",
			"%{prj.name}/src/**.def"
		}

		includedirs
		{
			"XYZEngine/vendor",
			"XYZEngine/vendor/yaml-cpp/include",
			"XYZEngine/src",
			"NativeScript",
			"%{IncludeDir.glm}",
			"%{IncludeDir.RCC}"
		}

		links
		{
			"XYZEngine"
		}

		filter "system:windows"
				systemversion "latest"

		filter "configurations:Debug"
				defines "XYZ_DEBUG"
				runtime "Debug"
				symbols "on"


		filter "configurations:Release"
				defines "XYZ_RELEASE"
				runtime "Release"
				optimize "on"


project "XYZScriptCore"
		location "XYZScriptCore"
		kind "SharedLib"
		language "C#"
			
		targetdir ("bin/" .. outputdir .. "/%{prj.name}")
		objdir ("bin-int/" .. outputdir .. "/%{prj.name}")
			
		files 
		{
			"%{prj.name}/src/**.cs", 
		}

