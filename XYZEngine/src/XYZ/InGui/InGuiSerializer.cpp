#include "stdafx.h"
#include "InGuiSerializer.h"

#include "XYZ/Utils/YamlUtils.h"

namespace XYZ {
	void InGuiSerializer::Serialize(const InGuiContext& context, const std::string& filepath)
	{
		YAML::Emitter out;
		out << YAML::BeginMap;
		out << YAML::Key << "Windows";
		out << YAML::Value << YAML::BeginSeq;
		for (auto window : context.m_Windows)
		{
			out << YAML::BeginMap;
			out << YAML::Key << "Name" << YAML::Value << window->Name;
			out << YAML::Key << "Position" << YAML::Value << window->Position;
			out << YAML::Key << "Size" << YAML::Value << window->Size;
			out << YAML::Key << "Collapsed" << YAML::Value << IS_SET(window->EditFlags, InGuiWindowEditFlags::Collapsed);
			out << YAML::EndMap;
		}
		out << YAML::EndSeq;
		out << YAML::EndMap;
		std::ofstream fout(filepath);
		fout << out.c_str();
	}
	void InGuiSerializer::Deserialize(InGuiContext& context, const std::string& filepath)
	{
		std::ifstream stream(filepath);
		std::stringstream strStream;
		strStream << stream.rdbuf();
		YAML::Node data = YAML::Load(strStream.str());
		auto windows = data["Windows"];
		for (auto& window : windows)
		{
			InGuiWindow* win = context.CreateInGuiWindow(window["Name"].as<std::string>().c_str());
			win->Position = window["Position"].as<glm::vec2>();
			win->Size = window["Size"].as<glm::vec2>();
			bool collapsed = window["Collapsed"].as<bool>();
			if (collapsed)
				win->EditFlags |= InGuiWindowEditFlags::Collapsed;
		}
	}
}