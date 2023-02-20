project "XYZPluginGenerator"
		kind "ConsoleApp"
		language "C#"
		
		Definitions = {}
		Definitions["TargetDirectory"] = "%{wks.location}/bin/" .. outputdir .. "/%{prj.name}"

		targetdir("%{Definitions.TargetDirectory}")
		objdir ("%{wks.location}/bin-int/" .. outputdir .. "/%{prj.name}")
			
		files 
		{
			"src/**.cs", 
		}
		buildcommands 
		{
			"dotnet add package YamlDotNet --version 13.0.1"
		}
		
		nuget
		{
			"System.CodeDom:7.0.0",
			"YamlDotNet:13.0.1"
		}

		links
		{
			"System"
		}

		dotnetframework("4.7.2")
