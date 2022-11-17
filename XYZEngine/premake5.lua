project "XYZEngine"
		filter "options:static"
			kind "StaticLib"
		filter "options:sharedexport"
			kind "SharedLib"
		filter "options:sharedimport"
			kind "SharedLib"
		filter{}
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

			"vendor/VulkanMemoryAllocator/**.h",
			"vendor/VulkanMemoryAllocator/**.cpp"

		}

		filter "options:sharedimport"
			defines
			{
				"XYZ_API_IMPORT"
			}
		filter "options:sharedexport"
			defines
			{
				"XYZ_API_EXPORT"
			}
			
		filter{}

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
			"%{IncludeDir.cr}",
			"%{IncludeDir.entt}",
			"%{IncludeDir.ozz_animation}",
			"%{IncludeDir.Assimp}",
			"%{IncludeDir.delaunator}",
			"%{IncludeDir.ImGui}",
			"%{IncludeDir.ImGuiNode}",
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
			"ImGui",
			"ImGuiNode",
			"box2d",
			"optick",
			"ozz_base",
			"ozz_animation",
			"ozz_animation_offline",   
			"%{LibraryDir.mono}",
			"%{Library.Vulkan}",
			"%{Library.VulkanUtils}"

		}
		
		flags { "NoPCH" }
		
		filter "system:windows"
				systemversion "latest"

		disablewarnings
		{
			"4251"
		}
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
					"%{Library.Assimp_Debug}",
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
					"%{Library.Assimp_Release}",
					"%{Library.ShaderC_Release}",
					"%{Library.SPIRV_Cross_Release}",
					"%{Library.SPIRV_Cross_GLSL_Release}"
				}