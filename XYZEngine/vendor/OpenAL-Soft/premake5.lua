project "OpenAL-Soft"
		kind "StaticLib"
		language "C++"
		cppdialect "C++14"
		staticruntime "off"

		targetdir ("bin/" .. outputdir .. "/%{prj.name}")
		objdir ("bin-int/" .. outputdir .. "/%{prj.name}")

	includedirs
	{
			"src",
			"src/al",
			"src/alc",
			"src/common",
			"include",
			"include/AL"
	}

	files
	{
			"src/**.h",
			"src/**.cpp",
			"include/**.h",
			"include/**.cpp"
	}

	excludes
	{
			"src/alc/mixer/mixer_neon.cpp"
	}

	defines
	{
			"AL_LIBTYPE_STATIC"
	}
	
	filter "system:windows"
	systemversion "latest"

		defines
		{
			"WIN32",
			"_WINDOWS",
			"AL_BUILD_LIBRARY",
			"AL_ALEXT_PROTOTYPES",
			"_WIN32",
			"_WIN32_WINNT=0x0502",
			"_CRT_SECURE_NO_WARNINGS",
			"NOMINMAX",
			"CMAKE_INTDIR=\"Debug\"",
			"OpenAL_EXPORTS"
		}

		links
		{
			"winmm"
		}

	filter "configurations:Debug"
		runtime "Debug"
		symbols "on"
		warnings "off"

	filter "configurations:Release"
		runtime "Release"
		optimize "on"
		warnings "off"
