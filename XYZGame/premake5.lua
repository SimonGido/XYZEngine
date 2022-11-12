project "XYZGame"
		kind "ConsoleApp"
		language "C++"
		cppdialect "C++17"
		staticruntime "off"
		
		targetdir ("%{wks.location}/bin/" .. outputdir .. "/%{prj.name}")
		objdir ("%{wks.location}/bin-int/" .. outputdir .. "/%{prj.name}")
		
		files
		{
			"src/**.h",
			"src/**.cpp"
		}
		
		includedirs
		{
			"src",
			"%{wks.location}/XYZEngine/vendor/spdlog/include",
			"%{wks.location}/XYZEngine/vendor",
			"%{wks.location}/XYZEngine/src",
			"%{IncludeDir.entt}",
			"%{IncludeDir.ozz_animation}",
			"%{IncludeDir.ImGui}",
			"%{IncludeDir.ImGuizmo}",
			"%{IncludeDir.yaml}",
			"%{IncludeDir.glm}",
			"%{IncludeDir.Asio}",
			"%{IncludeDir.box2d}",
			"%{IncludeDir.Vulkan}",
			"%{IncludeDir.optick}"
		}
		
		links
		{
			"ImGui"
		}
		
		filter "system:windows"
				systemversion "latest"
		
		filter "configurations:Debug"
				defines "XYZ_DEBUG"
				runtime "Debug"
				symbols "on"
		
		postbuildcommands 
		{
			'{COPY} "../XYZEngine/vendor/mono/bin/Debug/mono-2.0-sgen.dll" "%{cfg.targetdir}"',
			'{COPY} "%{Binaries.Assimp_Debug}" "%{cfg.targetdir}"',
			"{COPYDIR} \"%{LibraryDir.VulkanSDK_DebugDLL}\" \"%{cfg.targetdir}\""
		}
		
		filter "configurations:Release"
				defines "XYZ_RELEASE"
				runtime "Release"
				optimize "on"
		
		postbuildcommands 
		{
			'{COPY} "%{Binaries.Assimp_Release}" "%{cfg.targetdir}"',
			'{COPY} "../XYZEngine/vendor/mono/bin/Release/mono-2.0-sgen.dll" "%{cfg.targetdir}"'
		}