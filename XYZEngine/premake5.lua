project "XYZEngine"
		kind "StaticLib"
		language "C++"
		cppdialect "C++17"
		staticruntime "on"

		targetdir("bin/" .. outputdir .. "/%{prj.name}")
		objdir ("bin-int/" .. outputdir .. "/%{prj.name}")

		pchheader "stdafx.h"
		pchsource "src/stdafx.cpp"

		files
		{
			"src/XYZ.h",
			"src/stdafx.h",
			"src/stdafx.cpp",
			"src/XYZ/**.h",
			"src/XYZ/**.cpp",


			"vendor/glm/glm/**.hpp",
			"vendor/glm/glm/**.inl",
			"vendor/stb_image/**.h",
			"vendor/stb_image/**.cpp",

			"vendor/yaml-cpp/src/**.cpp",
			"vendor/yaml-cpp/src/**.h",
			"vendor/yaml-cpp/include/**.h",

			"vendor/asio/include/**.hpp",
			"vendor/asio/include/**.h",

			"vendor/ImGuizmo/ImGuizmo.h",
			"vendor/ImGuizmo/ImGuizmo.cpp",

			"vendor/ImGuizmo/ImCurveEdit.h",
			"vendor/ImGuizmo/ImCurveEdit.cpp",

			"vendor/ImGuizmo/ImSequencer.h",
			"vendor/ImGuizmo/ImSequencer.cpp"
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
			"src",
			"vendor/stb_image",		
			"vendor/assimp/include",
			"%{IncludeDir.ImGui}",
			"%{IncludeDir.ImGuizmo}",
			"%{IncludeDir.yaml}",
			"%{IncludeDir.GLFW}",
			"%{IncludeDir.GLEW}",
			"%{IncludeDir.glm}",
			"%{IncludeDir.OpenAL}/include",
			"%{IncludeDir.OpenAL}/src",
			"%{IncludeDir.OpenAL}/src/common",
			"%{IncludeDir.MiniMp3}",	
			"%{IncludeDir.FreeType}/include",
			"%{IncludeDir.Asio}",
			"%{IncludeDir.Lua}",
			"%{IncludeDir.Sol}",
			"%{IncludeDir.mono}",
			"%{IncludeDir.TrianglePP}",
			"%{IncludeDir.box2d}"
		}

		links
		{
			"GLEW",
			"GLFW",
			"OpenAL-Soft",
			"FreeType",
			"Lua",
			"opengl32",
			"TrianglePP",
			"imgui",
			"box2d",
			"%{LibraryDir.mono}"
		}
		
		flags { "NoPCH" }
		
		filter "system:windows"
				systemversion "latest"
		files
		{
			"src/Platform/Windows/**.h",
			"src/Platform/Windows/**.cpp",
		}

		filter "configurations:Debug"
				defines "XYZ_DEBUG"
				runtime "Debug"
				symbols "on"


		filter "configurations:Release"
				defines "XYZ_RELEASE"
				runtime "Release"
				optimize "on"