project "XYZScriptCore"
		kind "SharedLib"
		language "C#"
			
		targetdir ("%{wks.location}/XYZEditor/Assets/Scripts")
		objdir ("bin-int/" .. outputdir .. "/%{prj.name}")
			
		files 
		{
			"src/**.cs", 
		}