#include "stdafx.h"
#include "ImGuiStylePanel.h"

#include "XYZ/ImGui/ImGuiLayer.h"
#include "XYZ/ImGui/ImGui.h"
#include "XYZ/ImGui/ImGuiStyleReflection.h"

#include "XYZ/Core/Application.h"
#include "XYZ/Utils/FileSystem.h"
#include "XYZ/Reflection/Reflection.h"

#include <imgui/imgui.h>

#include <filesystem>

namespace XYZ {
	namespace Editor {
	
		bool ValueControl(const char* stringID, bool& value)
		{
			std::string valueID = "##";
			valueID += stringID;
			bool result = false;

			UI::TableRow(stringID,
				[&]() { ImGui::Text(stringID); },
				[&]() { result = ImGui::Checkbox(valueID.c_str(), &value); });

			return result;
		}
		bool ValueControl(const char* stringID, float& value)
		{
			std::string valueID = "##";
			valueID += stringID;
			bool result = false;

			UI::TableRow(stringID,
				[&]() { ImGui::Text(stringID); },
				[&]() { result = ImGui::DragFloat(valueID.c_str(), &value, 0.05f); });
			if (value <= 0.0f)
				value = 0.1f;

			return result;
		}
		bool ValueControl(const char* stringID, int& value)
		{
			std::string valueID = "##";
			valueID += stringID;
			bool result = false;

			UI::TableRow(stringID,
				[&]() { ImGui::Text(stringID); },
				[&]() { result = ImGui::DragInt(valueID.c_str(), &value, 0.05f); });

			return result;
		}
		bool ValueControl(const char* stringID, ImVec2& value)
		{
			std::string valueID = "##";
			valueID += stringID;
			bool result = false;

			UI::TableRow(stringID,
				[&]() { ImGui::Text(stringID); },
				[&]() { result = ImGui::DragFloat2(valueID.c_str(), (float*)&value, 0.05f); });

			if (value.x <= 0.0f)
				value.x = 0.1f;

			if (value.y <= 0.0f)
				value.y = 0.1f;

			return result;
		}
		bool ValueControl(const char* stringID, ImVec4* value)
		{
			return false;
		}

		ImGuiStylePanel::ImGuiStylePanel(std::string name)
			:
			EditorPanel(std::move(name))
		{
			
		}

		ImGuiStylePanel::~ImGuiStylePanel()
		{
		}

		void ImGuiStylePanel::OnImGuiRender(bool& open)
		{
			ImGuiLayer* imguiLayer = Application::Get().GetImGuiLayer();
			
			ImGuiStyle& style = ImGui::GetStyle();
		
			if (ImGui::Begin("ImGui Style", &open))
			{
				if (ImGui::Button("Save"))
				{
					void* window = Application::Get().GetWindow().GetNativeWindow();
					std::string filepath = FileSystem::OpenFile(window, "imxyz");
					if (std::filesystem::exists(filepath))
						imguiLayer->SaveStyle(filepath);
				}
				ImGui::SameLine();
				if (ImGui::Button("Load"))
				{
					void* window = Application::Get().GetWindow().GetNativeWindow();
					std::string filepath = FileSystem::OpenFile(window, "imxyz");
					if (std::filesystem::exists(filepath))
						imguiLayer->LoadStyle(filepath);
				}
				ImGui::SameLine();
				if (ImGui::Checkbox("Autosave", &m_AutoSave))
				{
					if (m_AutoSave)
					{
						void* window = Application::Get().GetWindow().GetNativeWindow();
						std::string filepath = FileSystem::OpenFile(window, "imxyz");
						if (std::filesystem::exists(filepath))
							m_AutoSaveFile = filepath;
						else
							m_AutoSave = false;
					}
				}
				if (m_AutoSave)
				{
					ImGui::SameLine();
					ImGui::Text(m_AutoSaveFile.c_str());
				}

				bool modified = false;
				if (ImGui::BeginTable("Colors", 2, ImGuiTableFlags_SizingFixedSame))
				{
					for (uint32_t i = 0; i < ImGuiCol_COUNT; ++i)
					{
						UI::TableRow(ImGuiStyleReflection::ImGuiColorIDText[i],
							[i]() { ImGui::Text(ImGuiStyleReflection::ImGuiColorIDText[i]); },
							[&]() { modified |= ImGui::ColorEdit4("##ColorEdit", (float*)&style.Colors[i]); });
					}
					ImGui::EndTable();
				}		
				if (ImGui::BeginTable("Variables", 2, ImGuiTableFlags_SizingFixedSame))
				{		
					auto styleTuple = ImGuiStyleReflection::StyleToTuple();
					Reflect::For<ImGuiStyleReflection::ImGuiStyleVariableCount>([&](auto j) 
					{
						modified |= ValueControl(ImGuiStyleReflection::ImGuiStyleVariables[j.value], std::get<j.value>(styleTuple));
					});

					handleLimits();
					ImGui::EndTable();
				}
				if (modified && m_AutoSave)
				{
					imguiLayer->SaveStyle(m_AutoSaveFile);
				}
			}
			ImGui::End();
		}
		void ImGuiStylePanel::handleLimits() const
		{
			ImGuiStyle& style = ImGui::GetStyle();
			if (style.Alpha > 1.0f)
				style.Alpha = 1.0f;

			if (style.DisabledAlpha > 1.0f)
				style.DisabledAlpha = 1.0f;
			
			if (style.WindowMenuButtonPosition < ImGuiDir_None)
				style.WindowMenuButtonPosition = ImGuiDir_None;
			if (style.WindowMenuButtonPosition >= ImGuiDir_COUNT)
				style.WindowMenuButtonPosition = ImGuiDir_COUNT - 1;
			
			if (style.ColorButtonPosition < ImGuiDir_None)
				style.ColorButtonPosition = ImGuiDir_None;
			if (style.ColorButtonPosition >= ImGuiDir_COUNT)
				style.ColorButtonPosition = ImGuiDir_COUNT - 1;
		}
	}
}

