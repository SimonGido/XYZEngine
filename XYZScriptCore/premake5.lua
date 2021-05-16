project "XYZScriptCore"
		kind "SharedLib"
		language "C#"
			
		targetdir ("XYZEditor/Assets/Scripts")
		objdir ("bin-int/" .. outputdir .. "/%{prj.name}")
			
		files 
		{
			"%{prj.name}/src/**.cs", 
		}