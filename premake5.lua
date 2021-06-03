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

IncludeDir = {}
IncludeDir["GLFW"] = "%{wks.location}/XYZEngine/vendor/glfw/include"
IncludeDir["GLEW"] = "%{wks.location}/XYZEngine/vendor/GLEW/include"
IncludeDir["glm"] = "%{wks.location}/XYZEngine/vendor/glm"
IncludeDir["OpenAL"] = "%{wks.location}/XYZEngine/vendor/OpenAL-Soft"
IncludeDir["MiniMp3"] = "%{wks.location}/XYZEngine/vendor/minimp3"
IncludeDir["FreeType"] = "%{wks.location}/XYZEngine/vendor/freetype-2.10.1"
IncludeDir["Asio"] = "%{wks.location}/XYZEngine/vendor/asio/include"
IncludeDir["Lua"] = "%{wks.location}/XYZEngine/vendor/lua/include"
IncludeDir["Sol"] = "%{wks.location}/XYZEngine/vendor/sol2/include"
IncludeDir["mono"] = "%{wks.location}/XYZEngine/vendor/mono/include"
IncludeDir["TrianglePP"] = "%{wks.location}/XYZEngine/vendor/TrianglePP/source"
IncludeDir["yaml"] = "%{wks.location}/XYZEngine/vendor/yaml-cpp/include"
IncludeDir["ImGui"] = "%{wks.location}/XYZEngine/vendor/imgui"

LibraryDir = {}
LibraryDir["mono"] = "vendor/mono/lib/Debug/mono-2.0-sgen.lib"

group "Dependencies"
		include "XYZEngine/vendor/glfw"
		include "XYZEngine/vendor/GLEW"
		include "XYZEngine/vendor/OpenAL-Soft"
		include "XYZEngine/vendor/freetype-2.10.1"
		include "XYZEngine/vendor/lua"
		include "XYZEngine/vendor/TrianglePP"
		include "XYZEngine/vendor/imgui"
group ""

include "XYZEngine"
include "XYZEditor"
include "XYZSandbox"			
include "XYZServer"
include "XYZClient"
include "XYZScriptCore"
include "XYZScriptExample"