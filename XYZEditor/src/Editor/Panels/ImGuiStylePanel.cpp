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
		bool ValueControl(const char* stringID, ImVec4(&values)[ImGuiCol_COUNT])
		{
			bool result = false;
			for (uint32_t i = 0; i < ImGuiCol_COUNT; ++i)
			{
				ImVec4& value = values[i];
				UI::TableRow(ImGuiStyleReflection::ImGuiColorIDText[i],
					[i]() { ImGui::Text(ImGuiStyleReflection::ImGuiColorIDText[i]); },
					[&]() { result |= ImGui::ColorEdit4("##ColorEdit", (float*)&value); });
			}
			return result;
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
				ImGui::NewLine();
				bool modified = false;
				if (ImGui::BeginTabBar("##tabs", ImGuiTabBarFlags_None))
				{
					modified |= styleTab();
					modified |= fontTab();

					ImGui::EndTabBar();
				}

				if (modified && m_AutoSave)
					imguiLayer->SaveStyle(m_AutoSaveFile);
			}
			ImGui::End();
		}
		
		bool ImGuiStylePanel::styleTab() const
		{
			bool modified = false;
			if (ImGui::BeginTabItem("Style"))
			{
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
				ImGui::EndTabItem();
			}
			return modified;
		}
		bool ImGuiStylePanel::fontTab()
		{
			bool modified = false;
			if (ImGui::BeginTabItem("Font"))
			{
				handleAddFont();
				ImGui::Separator();
				ImGuiIO& io = ImGui::GetIO();
				ImFontAtlas* atlas = io.Fonts;

				const ImGuiTableFlags tableFlags = ImGuiTableFlags_SizingFixedFit;
				if (ImGui::BeginTable("##FontTable", 2, tableFlags))
				{
					for (int i = 0; i < atlas->Fonts.Size; i++)
					{
						ImFont* font = atlas->Fonts[i];
						std::string fontName = font->ConfigData ? font->ConfigData[0].Name : "";

						UI::TableRow(font,
							[&]() {
							bool isDefault = font == io.FontDefault;
							if (ImGui::Checkbox("##SetDefault", &isDefault))
								io.FontDefault = font;
						},
							[]() {});
						UI::TableRow(font,
							[]() { ImGui::Text("Name"); },
							[&]() { 
							UI::ScopedStyleStack style(ImGuiStyleVar_Alpha, 0.6f);
							UI::ScopedTableColumnAutoWidth scoped(1);
							ImGui::InputText("##Name", (char*)fontName.c_str(), fontName.size(), ImGuiInputTextFlags_ReadOnly); });
					
						UI::TableRow(font,
							[]() { ImGui::Text("Font Size"); },
							[&]() {
							UI::ScopedStyleStack style(ImGuiStyleVar_Alpha, 0.6f);
							UI::ScopedTableColumnAutoWidth scoped(1);
							ImGui::InputFloat("##FontSize", &font->FontSize, 0.0f, 0.0f, "%.2f", ImGuiInputTextFlags_ReadOnly); });

						UI::TableRow(font,
							[]() { ImGui::Text("Glyph Count"); },
							[&]() {
							UI::ScopedStyleStack style(ImGuiStyleVar_Alpha, 0.6f);
							UI::ScopedTableColumnAutoWidth scoped(1);
							ImGui::InputInt("##GlyphCount", &font->Glyphs.Size, 0.0f, 0.0f, ImGuiInputTextFlags_ReadOnly); });
						
						UI::TableRow(font,
							[]() { ImGui::Text("Preview"); },
							[&]() {
							ImGui::PushFont(font);
							ImGui::Text("The quick brown fox jumps over the lazy dog");
							ImGui::PopFont(); });
						
						

						ImGui::NewLine();
						ImGui::NewLine();
					}
					ImGui::EndTable();
				}
				ImGui::EndTabItem();
			}
			return modified;
		}

		void ImGuiStylePanel::handleAddFont()
		{
			ImGuiIO& io = ImGui::GetIO();
			const ImGuiTableFlags tableFlags = ImGuiTableFlags_SizingStretchProp;
			
			if (ImGui::BeginTable("##AddFontTable", 2, tableFlags))
			{
				UI::TableRow("InputPath",
					[]() { ImGui::Text("Font Path"); },
					[&]() {
						ImGui::InputTextWithHint("##FontPath", "Select...", (char*)m_LoadFontPath.c_str(), m_LoadFontPath.size(), ImGuiInputTextFlags_ReadOnly);
						if (ImGui::IsItemClicked(ImGuiMouseButton_Left))
						{
							void* window = Application::Get().GetWindow().GetNativeWindow();
							m_LoadFontPath = FileSystem::OpenFile(window);
							m_LoadFontPath = std::filesystem::relative(m_LoadFontPath).string();
							std::replace(m_LoadFontPath.begin(), m_LoadFontPath.end(), '\\', '/');
						}		
					});
				UI::TableRow("FontSize",
					[]() { ImGui::Text("Font Size"); },
					[&]() { ImGui::InputFloat("##FontSize", &m_LoadFontSize); });

				UI::TableRow("FontSize",
					[]() { },
					[&]() {
					bool validPath = std::filesystem::exists(m_LoadFontPath);
					UI::ScopedItemFlags flags(ImGuiItemFlags_Disabled, !validPath);
					if (ImGui::Button("Add Font"))
					{
						ImGuiLayer* imguiLayer = Application::Get().GetImGuiLayer();
						imguiLayer->AddFont({ m_LoadFontPath, m_LoadFontSize, io.Fonts->GetGlyphRangesCyrillic() });
					}		
				});
				ImGui::EndTable();
			}
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

