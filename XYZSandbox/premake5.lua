project "XYZSandbox"
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
			"%{wks.location}/XYZEngine/vendor/spdlog/include",
			"%{wks.location}/XYZEngine/vendor/yaml-cpp/include",
			"%{wks.location}/XYZEngine/vendor",
			"%{wks.location}/XYZEngine/src",
			"%{IncludeDir.ImGui}",
			"%{IncludeDir.ImGuizmo}",
			"%{IncludeDir.glm}",
			"%{IncludeDir.Asio}",
			"%{IncludeDir.Lua}",
			"%{IncludeDir.Sol}",
			"%{IncludeDir.box2d}"
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

		postbuildcommands 
		{
			'{COPY} "../XYZEngine/vendor/mono/bin/Release/mono-2.0-sgen.dll" "%{cfg.targetdir}"'
		}