project "XYZNative"
		kind "SharedLib"
		language "C++"
		cppdialect "C++17"
		staticruntime "off"
		
		targetdir ("%{wks.location}/bin/" .. outputdir .. "/%{prj.name}")
		objdir ("%{wks.location}/bin-int/" .. outputdir .. "/%{prj.name}")
		
		files
		{
			"**.h",
			"**.cpp",
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
			"%{IncludeDir.ImGuiNode}",
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
			
			"%{wks.location}/bin/" .. outputdir .."/XYZEngine/XYZEngine.lib"
		}
		
		
		filter "system:windows"
				systemversion "latest"
		
		filter "configurations:Debug"
				runtime "Debug"
				symbols "on"
						
		
		filter "configurations:Release"
				runtime "Release"
				optimize "on"