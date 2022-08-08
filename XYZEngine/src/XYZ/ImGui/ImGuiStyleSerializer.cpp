#include "stdafx.h"
#include "ImGuiStyleSerializer.h"

#include "XYZ/Core/Application.h"
#include "XYZ/ImGui/ImGuiStyleReflection.h"
#include "XYZ/ImGui/ImGui.h"

#include "XYZ/Reflection/Reflection.h"
#include "XYZ/Utils/YamlUtils.h"

namespace XYZ {
	
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

	void ImGuiStyleSerializer::saveStyle(const std::string& filepath)
	{
		ImGuiStyle& style = ImGui::GetStyle();
		const ImGuiLayer* imguiLayer = Application::Get().GetImGuiLayer();
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
			out << YAML::Key << "Color" << YAML::Value << (glm::vec4)linearColor;
			out << YAML::EndMap;
		}
		out << YAML::EndSeq;

		//////////////////////
		out << YAML::Key << "Variables";
		out << YAML::Value << YAML::BeginSeq;
		auto styleTuple = ImGuiStyleReflection::StyleToTuple();
		Utils::For<ImGuiStyleReflection::ImGuiStyleVariableCount>([&](auto j)
		{
			out << YAML::BeginMap;
			out << YAML::Key << "Index" << YAML::Value << j.value;
			out << YAML::Key << ImGuiStyleReflection::ImGuiStyleVariables[j.value] << YAML::Value << std::get<j.value>(styleTuple);
			out << YAML::EndMap;
		});
		out << YAML::EndSeq;
		//////////////////////
		//////////////////////
		out << YAML::Key << "Fonts";
		out << YAML::Value << YAML::BeginSeq;
		for (const auto& fontConfig : imguiLayer->GetLoadedFonts())
		{
			out << YAML::BeginMap;
			out << YAML::Key << "FilePath" << YAML::Value << fontConfig.Filepath;
			out << YAML::Key << "SizePixels" << YAML::Value << fontConfig.SizePixels;
			out << YAML::EndMap;
		}
		out << YAML::EndSeq;
		out << YAML::EndMap; // Fonts

		std::ofstream fout(filepath);
		fout << out.c_str();
	}
	void ImGuiStyleSerializer::loadStyle(const std::string& filepath)
	{
		std::ifstream stream(filepath);
		std::stringstream strStream;
		if (!stream)
			XYZ_CORE_ERROR("Could not load imgui style {0}", strerror(errno));
		strStream << stream.rdbuf();
		YAML::Node data = YAML::Load(strStream.str());

		ImGuiStyle& style = ImGui::GetStyle();
		ImGuiLayer* imguiLayer = Application::Get().GetImGuiLayer();

		auto styleTuple = ImGuiStyleReflection::StyleToTuple();
		auto colors = data["Colors"];
		for (auto& color : colors)
		{
			uint32_t index = color["Index"].as<uint32_t>();
			style.Colors[index] = (ImVec4)color["Color"].as<glm::vec4>();
		}
		auto variables = data["Variables"];
		for (auto& var : variables)
		{
			uint32_t index = var["Index"].as<uint32_t>();
			Utils::For<ImGuiStyleReflection::ImGuiStyleVariableCount>([&](auto j)
			{
				if (j.value == index)
				{
					ParseValue(std::get<j.value>(styleTuple), var[ImGuiStyleReflection::ImGuiStyleVariables[index]]);
				}
			});
		}
		auto fonts = data["Fonts"];
		for (auto& font : fonts)
		{
			std::string filePath = font["FilePath"].as<std::string>();
			float sizePixels = font["SizePixels"].as<float>();
			imguiLayer->AddFont({ filepath, sizePixels, ImGui::GetIO().Fonts->GetGlyphRangesCyrillic() });
		}

		if (m_SRGBColorSpace)
		{
			for (uint32_t i = 0; i < ImGuiCol_COUNT; ++i)
				style.Colors[i] = UI::Utils::ConvertToLinear(style.Colors[i]);
		}
	}
}