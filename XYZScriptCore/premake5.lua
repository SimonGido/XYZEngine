project "XYZScriptCore"
		kind "SharedLib"
		language "C#"
			
		targetdir ("%{wks.location}/XYZEditor/Assets/Scripts")
		objdir ("%{wks.location}/bin-int/" .. outputdir .. "/%{prj.name}")
			
		files 
		{
			"src/**.cs", 
		}