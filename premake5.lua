newoption
{
	trigger = "static",
	description = "Build XYZEngine as static library"
}

newoption
{
	trigger = "sharedexport",
	description = "Build XYZEngine as shared library"
}

newoption
{
	trigger = "sharedimport",
	description = "Build XYZEngine as shared library"
}

workspace "XYZEngine"
		startproject "XYZEditor"
		
		configurations { "Debug", "Release" }

		filter "configurations:Debug"
			architecture "x64"
		filter "configurations:Release"
			architecture "x64"

		filter{}
		
		flags
		{
			"MultiProcessorCompile"
		}
		
outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"


include "Projects.lua"
