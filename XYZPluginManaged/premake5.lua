workspace "XYZPluginManaged"
configurations
{
"Debug",
"Release"
}
architecture "x64"

project "XYZPluginManaged"
	kind "SharedLib"
	language "C#"
	targetdir("bin/%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}")
	objdir("bin-int/%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}")

files
{
"**.h",
"**.cpp",
"**.cs"
}

includedirs
{
"C:/Users/Gido/XYZEngine/vendor",
"C:/Users/Gido/XYZEngine/src",
"C:/Users/Gido/XYZEngine/vendor/spdlog/include",
"C:/Users/Gido/XYZEngine/vendor/imgui",
"C:/Users/Gido/XYZEngine/vendor/ImGuizmo",
"C:/Users/Gido/XYZEngine/vendor/imgui-node-editor",
"C:/Users/Gido/XYZEngine/vendor/glm",
"C:/Users/Gido/XYZEngine/vendor/ozz-animation/include",
"C:/Users/Gido/XYZEngine/vendor/box2d/include",
"C:/Users/Gido/XYZEngine/vendor/entt/include",
"C:/Users/Gido/XYZEngine/vendor/OpenXR-SDK/include",
"C:/Users/Gido/XYZEngine/vendor/optick/src",
"C:/Users/Gido/XYZEngine/vendor/yaml-cpp/include",
"C:/Users/Gido/XYZEngine/vendor/asio/include"
}

links
{
"C:/Users/Gido/Projects/XYZEngine/XYZScriptCore/XYZScriptCore.dll"
}

