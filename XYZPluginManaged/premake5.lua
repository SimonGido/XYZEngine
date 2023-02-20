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
"C://Users//Gido//Projects//XYZEngine//XYZEngine/vendor",
"C://Users//Gido//Projects//XYZEngine//XYZEngine/src",
"C://Users//Gido//Projects//XYZEngine//XYZEngine/vendor/spdlog/include",
"C://Users//Gido//Projects//XYZEngine//XYZEngine/vendor/imgui",
"C://Users//Gido//Projects//XYZEngine//XYZEngine/vendor/ImGuizmo",
"C://Users//Gido//Projects//XYZEngine//XYZEngine/vendor/imgui-node-editor",
"C://Users//Gido//Projects//XYZEngine//XYZEngine/vendor/glm",
"C://Users//Gido//Projects//XYZEngine//XYZEngine/vendor/ozz-animation/include",
"C://Users//Gido//Projects//XYZEngine//XYZEngine/vendor/box2d/include",
"C://Users//Gido//Projects//XYZEngine//XYZEngine/vendor/entt/include",
"C://Users//Gido//Projects//XYZEngine//XYZEngine/vendor/OpenXR-SDK/include",
"C://Users//Gido//Projects//XYZEngine//XYZEngine/vendor/optick/src",
"C://Users//Gido//Projects//XYZEngine//XYZEngine/vendor/yaml-cpp/include",
"C://Users//Gido//Projects//XYZEngine//XYZEngine/vendor/asio/include"
}

links
{
"C:/Users/Gido/Projects/XYZEngine/XYZScriptCore/XYZScriptCore.dll"
}

