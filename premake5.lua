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
IncludeDir["mini"] = "XYZEngine/vendor/mini"
IncludeDir["OpenAL"] = "XYZEngine/vendor/OpenAL-Soft"
IncludeDir["MiniMp3"] = "XYZEngine/vendor/minimp3"
IncludeDir["FreeType"] = "XYZEngine/vendor/freetype-2.10.1"
IncludeDir["Asio"] = "XYZEngine/vendor/asio/include"
IncludeDir["Lua"] = "XYZEngine/vendor/lua/include"
IncludeDir["Sol"] = "XYZEngine/vendor/sol2/include"
IncludeDir["mono"] = "XYZEngine/vendor/mono/include"
IncludeDir["earcut"] = "XYZEngine/vendor/earcut/include"

LibraryDir = {}
LibraryDir["mono"] = "vendor/mono/lib/Debug/mono-2.0-sgen.lib"

include "XYZEngine/vendor/GLFW"
include "XYZEngine/vendor/GLEW"
include "XYZEngine/vendor/OpenAL-Soft"
include "XYZEngine/vendor/freetype-2.10.1"
include "XYZEngine/vendor/lua"

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
			"%{prj.name}/vendor/glm/glm/**.hpp",
			"%{prj.name}/vendor/glm/glm/**.inl",
			"%{prj.name}/vendor/mini/ini.h",
			"%{prj.name}/vendor/stb_image/**.h",
			"%{prj.name}/vendor/stb_image/**.cpp",

			"%{prj.name}/vendor/yaml-cpp/src/**.cpp",
			"%{prj.name}/vendor/yaml-cpp/src/**.h",
			"%{prj.name}/vendor/yaml-cpp/include/**.h",

			"%{prj.name}/vendor/asio/include/**.hpp",
			"%{prj.name}/vendor/asio/include/**.h"
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
			"%{IncludeDir.mini}",
			"%{IncludeDir.OpenAL}/include",
			"%{IncludeDir.OpenAL}/src",
			"%{IncludeDir.OpenAL}/src/common",
			"%{IncludeDir.MiniMp3}",	
			"%{prj.name}/vendor/stb_image",
			"%{prj.name}/vendor/yaml-cpp/include",
			"%{IncludeDir.FreeType}/include",
			"%{IncludeDir.Asio}",
			"%{IncludeDir.Lua}",
			"%{IncludeDir.Sol}",
			"%{IncludeDir.mono}",
			"%{IncludeDir.earcut}"
		}

		links
		{
			"GLEW",
			"GLFW",
			"OpenAL-Soft",
			"FreeType",
			"Lua",
			"opengl32",
			"%{LibraryDir.mono}"
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
			"%{prj.name}/src/**.cpp"
		}
		
		includedirs
		{
			"XYZEngine/vendor/yaml-cpp/include",
			"XYZEngine/vendor",
			"XYZEngine/src",
			"%{IncludeDir.glm}",
			"%{IncludeDir.Asio}",
			"%{IncludeDir.Lua}",
			"%{IncludeDir.Sol}"
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


project "XYZSandbox"
		location "XYZSandbox"
		kind "ConsoleApp"
		language "C++"
		cppdialect "C++17"
		staticruntime "on"
		
		targetdir ("bin/" .. outputdir .. "/%{prj.name}")
		objdir ("bin-int/" .. outputdir .. "/%{prj.name}")
		
		files
		{
			"%{prj.name}/src/**.h",
			"%{prj.name}/src/**.cpp"
		}
		
		includedirs
		{
			"XYZEngine/vendor/yaml-cpp/include",
			"XYZEngine/vendor",
			"XYZEngine/src",
			"%{IncludeDir.glm}",
			"%{IncludeDir.Asio}",
			"%{IncludeDir.Lua}",
			"%{IncludeDir.Sol}"
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

		postbuildcommands 
		{
			'{COPY} "../XYZEngine/vendor/mono/bin/Debug/mono-2.0-sgen.dll" "%{cfg.targetdir}"'
		}
		
		filter "configurations:Release"
				defines "XYZ_RELEASE"
				runtime "Release"
				optimize "on"

project "XYZServer"
		location "XYZServer"
		kind "ConsoleApp"
		language "C++"
		cppdialect "C++17"
		staticruntime "on"
		
		targetdir ("bin/" .. outputdir .. "/%{prj.name}")
		objdir ("bin-int/" .. outputdir .. "/%{prj.name}")
		
		files
		{
			"%{prj.name}/src/**.h",
			"%{prj.name}/src/**.cpp"
		}
		
		includedirs
		{
			"XYZEngine/vendor/yaml-cpp/include",
			"XYZEngine/vendor",
			"XYZEngine/src",
			"%{IncludeDir.glm}",
			"%{IncludeDir.Asio}",
			"%{IncludeDir.Lua}",
			"%{IncludeDir.Sol}"
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


project "XYZClient"
		location "XYZClient"
		kind "ConsoleApp"
		language "C++"
		cppdialect "C++17"
		staticruntime "on"
		
		targetdir ("bin/" .. outputdir .. "/%{prj.name}")
		objdir ("bin-int/" .. outputdir .. "/%{prj.name}")
		
		files
		{
			"%{prj.name}/src/**.h",
			"%{prj.name}/src/**.cpp"
		}
		
		includedirs
		{
			"XYZEngine/vendor/yaml-cpp/include",
			"XYZEngine/vendor",
			"XYZEngine/src",
			"%{IncludeDir.glm}",
			"%{IncludeDir.Asio}",
			"%{IncludeDir.Lua}",
			"%{IncludeDir.Sol}"
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
			
		targetdir ("XYZEditor/Assets/Scripts")
		objdir ("bin-int/" .. outputdir .. "/%{prj.name}")
			
		files 
		{
			"%{prj.name}/src/**.cs", 
		}

project "XYZScriptExample"
		location "XYZScriptExample"
		kind "SharedLib"
		language "C#"
			
		targetdir ("XYZEditor/Assets/Scripts")
		objdir ("bin-int/" .. outputdir .. "/%{prj.name}")
			
		files 
		{
			"%{prj.name}/src/**.cs", 
		}

		links
		{
			"XYZScriptCore"
		}