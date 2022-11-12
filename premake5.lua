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
		architecture "x64"
		startproject "XYZEditor"

		configurations
		{
			"Debug",
			"Release"
		}

		flags
		{
			"MultiProcessorCompile"
		}
		
outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"


include "Projects.lua"
