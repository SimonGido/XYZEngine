project "XYZPluginGenerator"
		kind "ConsoleApp"
		language "C#"
			
		targetdir ("%{wks.location}/bin/" .. outputdir .. "/%{prj.name}")
		objdir ("%{wks.location}/bin-int/" .. outputdir .. "/%{prj.name}")
			
		files 
		{
			"src/**.cs", 
		}

		nuget
		{
			"System.CodeDom:7.0.0"
		}

		links
		{
			"System"
		}
		
		dotnetframework("4.7.2")
