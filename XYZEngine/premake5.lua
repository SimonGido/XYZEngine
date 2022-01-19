project "XYZEngine"
		kind "StaticLib"
		language "C++"
		cppdialect "C++17"
		staticruntime "off"

		targetdir("%{wks.location}/bin/" .. outputdir .. "/%{prj.name}")
		objdir ("%{wks.location}/bin-int/" .. outputdir .. "/%{prj.name}")

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
			"vendor/ImGuizmo/ImSequencer.cpp",

			"vendor/VulkanMemoryAllocator/**.h",
			"vendor/VulkanMemoryAllocator/**.cpp"

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
			"vendor",
			"vendor/stb_image",		
			"vendor/assimp/include",
			"vendor/spdlog/include",
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
			"%{IncludeDir.mono}",
			"%{IncludeDir.TrianglePP}",
			"%{IncludeDir.box2d}",
			"%{IncludeDir.optick}",
			"%{IncludeDir.VulkanSDK}"
		}

		links
		{
			"GLEW",
			"GLFW",
			"OpenAL-Soft",
			"FreeType",
			"opengl32",
			"TrianglePP",
			"ImGui",
			"box2d",
			"optick",
			"%{LibraryDir.mono}",
			"%{Library.Vulkan}",
			"%{Library.VulkanUtils}"

		}
		
		flags { "NoPCH" }
		
		filter "system:windows"
				systemversion "latest"
		files
		{
			"src/XYZ/Platform/Windows/**.h",
			"src/XYZ/Platform/Windows/**.cpp",
		}

		filter "configurations:Debug"
				defines "XYZ_DEBUG"
				runtime "Debug"
				symbols "on"

				links
				{
					"%{Library.ShaderC_Debug}",
					"%{Library.SPIRV_Cross_Debug}",
					"%{Library.SPIRV_Cross_GLSL_Debug}"
				}


		filter "configurations:Release"
				defines "XYZ_RELEASE"
				runtime "Release"
				optimize "on"
				
				links
				{
					"%{Library.ShaderC_Release}",
					"%{Library.SPIRV_Cross_Release}",
					"%{Library.SPIRV_Cross_GLSL_Release}"
				}