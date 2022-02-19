include "Dependencies.lua"

group "Dependencies"
		include "XYZEngine/vendor/glfw"
		include "XYZEngine/vendor/GLEW"
		include "XYZEngine/vendor/OpenAL-Soft"
		include "XYZEngine/vendor/freetype-2.10.1"
		include "XYZEngine/vendor/imgui"
		include "XYZEngine/vendor/box2d"
		include "XYZEngine/vendor/optick"
group ""

include "XYZEngine"
include "XYZEditor"		
include "XYZServer"
include "XYZClient"
include "XYZScriptCore"
include "XYZScript"