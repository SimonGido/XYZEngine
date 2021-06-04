project "XYZScriptExample"
		kind "SharedLib"
		language "C#"
			
		targetdir ("%{wks.location}/XYZEditor/Assets/Scripts")
		objdir ("bin-int/" .. outputdir .. "/%{prj.name}")
			
		files 
		{
			"src/**.cs",
			"%{wks.location}/XYZEditor/Assets/**.cs"
		}

		links
		{
			"XYZScriptCore"
		}