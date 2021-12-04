#include "stdafx.h"
#include "ImGuiStyleSerializer.h"

#include "XYZ/ImGui/ImGuiStyleReflection.h"
#include "XYZ/ImGui/ImGui.h"

#include "XYZ/Reflection/Reflection.h"
#include "XYZ/Utils/YamlUtils.h"

namespace XYZ {
	static glm::vec4 ConvertImVec4ToGlm(const ImVec4& val)
	{
		return glm::vec4{ val.x, val.y, val.z ,val.w };
	}
	static ImVec4 ConvertGlmVec4ToIm(const glm::vec4& val)
	{
		return ImVec4{ val.x, val.y, val.z ,val.w };
	}

	static void ParseValue(float& val, const YAML::Node& node)
	{
		val = node.as<float>();
	}
	static void ParseValue(ImVec2& val, const YAML::Node& node)
	{
		val = node.as<ImVec2>();
	}
	static void ParseValue(bool& val, const YAML::Node& node)
	{
		val = node.as<bool>();
	}

	static void ParseValue(int& val, const YAML::Node& node)
	{
		val = node.as<int>();
	}

	static void ParseValue(ImVec4* val, const YAML::Node& node)
	{
		// DO nothing
	}

	ImGuiStyleSerializer::ImGuiStyleSerializer(bool srgb)
		:
		m_SRGBColorSpace(srgb)
	{
	}

	void ImGuiStyleSerializer::SaveStyle(const std::string& filepath)
	{
		ImGuiStyle& style = ImGui::GetStyle();

		YAML::Emitter out;
		out << YAML::BeginMap; // Style

		//////////////////////
		out << YAML::Key << "Colors";
		out << YAML::Value << YAML::BeginSeq;
		for (uint32_t i = 0; i < ImGuiCol_COUNT; ++i)
		{
			ImVec4 linearColor = m_SRGBColorSpace ? UI::Utils::ConvertToSRGB(style.Colors[i]) : style.Colors[i];
			out << YAML::BeginMap;
			out << YAML::Key << "Index" << YAML::Value << i;
			out << YAML::Key << "Color" << YAML::Value << ConvertImVec4ToGlm(linearColor);
			out << YAML::EndMap;
		}
		out << YAML::EndSeq;

		//////////////////////
		out << YAML::Key << "Variables";
		out << YAML::Value << YAML::BeginSeq;
		auto styleTuple = ImGuiStyleReflection::StyleToTuple();
		Reflect::For<ImGuiStyleReflection::ImGuiStyleVariableCount>([&](auto j)
		{
			out << YAML::BeginMap;
			out << YAML::Key << "Index" << YAML::Value << j.value;
			out << YAML::Key << ImGuiStyleReflection::ImGuiStyleVariables[j.value] << YAML::Value << std::get<j.value>(styleTuple);
			out << YAML::EndMap;
		});
		out << YAML::EndSeq;
		//////////////////////


		out << YAML::EndMap; // Style

		std::ofstream fout(filepath);
		fout << out.c_str();
	}
	void ImGuiStyleSerializer::LoadStyle(const std::string& filepath)
	{
		std::ifstream stream(filepath);
		std::stringstream strStream;
		if (!stream)
			XYZ_ERROR("Could not load imgui style {0}", strerror(errno));
		strStream << stream.rdbuf();
		YAML::Node data = YAML::Load(strStream.str());

		ImGuiStyle& style = ImGui::GetStyle();
		auto styleTuple = ImGuiStyleReflection::StyleToTuple();
		auto colors = data["Colors"];
		for (auto& color : colors)
		{
			uint32_t index = color["Index"].as<uint32_t>();
			style.Colors[index] = ConvertGlmVec4ToIm(color["Color"].as<glm::vec4>());
		}
		auto variables = data["Variables"];
		for (auto& var : variables)
		{
			uint32_t index = var["Index"].as<uint32_t>();
			Reflect::For<ImGuiStyleReflection::ImGuiStyleVariableCount>([&](auto j)
			{
				if (j.value == index)
				{
					ParseValue(std::get<j.value>(styleTuple), var[ImGuiStyleReflection::ImGuiStyleVariables[index]]);
				}
			});
		}
		if (m_SRGBColorSpace)
		{
			for (uint32_t i = 0; i < ImGuiCol_COUNT; ++i)
				style.Colors[i] = UI::Utils::ConvertToLinear(style.Colors[i]);
		}
	}
}