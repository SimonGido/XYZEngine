project "Lua"
    	kind "StaticLib"
		language "C"
		staticruntime "on"
    
	targetdir ("bin/" .. outputdir .. "/%{prj.name}")
    objdir ("bin-int/" .. outputdir .. "/%{prj.name}")

	files
	{
		"include/**.h",
		"include/**.hpp",
        "src/**.hpp",
        "src/**.h",
		"src/**.c",	
    }
    

	filter "configurations:Debug"
			runtime "Debug"
			symbols "on"

	filter "configurations:Release"
			runtime "Release"
			optimize "on"