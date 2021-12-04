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
	
		void ValueControl(const char* stringID, bool& value)
		{
			std::string valueID = "##";
			valueID += stringID;

			UI::TableRow(stringID,
				[&]() { ImGui::Text(stringID); },
				[&]() { ImGui::Checkbox(valueID.c_str(), &value); });
		}
		void ValueControl(const char* stringID, float& value)
		{
			std::string valueID = "##";
			valueID += stringID;

			UI::TableRow(stringID,
				[&]() { ImGui::Text(stringID); },
				[&]() { ImGui::InputFloat(valueID.c_str(), &value); });
		}
		void ValueControl(const char* stringID, int& value)
		{
			std::string valueID = "##";
			valueID += stringID;

			UI::TableRow(stringID,
				[&]() { ImGui::Text(stringID); },
				[&]() { ImGui::InputInt(valueID.c_str(), &value); });
		}
		void ValueControl(const char* stringID, ImVec2& value)
		{
			std::string valueID = "##";
			valueID += stringID;

			UI::TableRow(stringID,
				[&]() { ImGui::Text(stringID); },
				[&]() { ImGui::InputFloat2(valueID.c_str(), (float*)&value); });
		}
		void ValueControl(const char* stringID, ImVec4* value)
		{
			// DO nothing
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

				if (ImGui::BeginTable("Colors", 2, ImGuiTableFlags_SizingFixedSame))
				{
					for (uint32_t i = 0; i < ImGuiCol_COUNT; ++i)
					{
						UI::TableRow(ImGuiStyleReflection::ImGuiColorIDText[i],
							[i]() { ImGui::Text(ImGuiStyleReflection::ImGuiColorIDText[i]); },
							[&]() { ImGui::ColorEdit4("##ColorEdit", (float*)&style.Colors[i]); });
					}
					ImGui::EndTable();
				}		
				if (ImGui::BeginTable("Variables", 2, ImGuiTableFlags_SizingFixedSame))
				{		
					auto styleTuple = ImGuiStyleReflection::StyleToTuple();
					Reflect::For<ImGuiStyleReflection::ImGuiStyleVariableCount>([&](auto j) 
					{
						ValueControl(ImGuiStyleReflection::ImGuiStyleVariables[j.value], std::get<j.value>(styleTuple));
					});

					if (style.Alpha < 0.1f)
						style.Alpha = 0.1f;
					
					ImGui::EndTable();
				}
			}
			ImGui::End();
		}
	}
}

