project "XYZServer"
		kind "ConsoleApp"
		language "C++"
		cppdialect "C++17"
		staticruntime "on"
		
		targetdir ("bin/" .. outputdir .. "/%{prj.name}")
		objdir ("bin-int/" .. outputdir .. "/%{prj.name}")
		
		files
		{
			"src/**.h",
			"src/**.cpp"
		}
		
		includedirs
		{
			"%{wks.location}/XYZEngine/vendor/yaml-cpp/include",
			"%{wks.location}/XYZEngine/vendor",
			"%{wks.location}/XYZEngine/src",
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