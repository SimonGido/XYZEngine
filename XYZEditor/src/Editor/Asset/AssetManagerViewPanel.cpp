#include "stdafx.h"
#include "AssetManagerViewPanel.h"
#include "XYZ/Asset/AssetManager.h"

#include "XYZ/ImGui/ImGui.h"
#include "XYZ/Utils/StringUtils.h"

#include <imgui/imgui.h>

namespace XYZ {
	namespace Editor {
		AssetManagerViewPanel::AssetManagerViewPanel(std::string name)
			:
			EditorPanel(std::move(name))
		{
		}

		AssetManagerViewPanel::~AssetManagerViewPanel()
		{
			
		}

		void AssetManagerViewPanel::OnImGuiRender(bool& open)
		{
			if (ImGui::Begin("Asset View", &open))
			{
				MemoryPoolView view(AssetManager::GetMemoryPool());
				view.OnImGuiRender();

				if (UI::BeginTreeNode("Asset Metadata"))
				{
					static char searchBuffer[_MAX_PATH];
					ImGui::InputTextWithHint("##search", "Search...", searchBuffer, _MAX_PATH);

					const ImGuiTableFlags tableFlags = ImGuiTableFlags_SizingStretchProp;

					if (ImGui::BeginTable("##AssetMetadata", 2, tableFlags))
					{
						if (searchBuffer[0] != 0)
						{
							std::string searchString = searchBuffer;
							displaySearchedMetadata(searchString);
						}
						else
						{
							displayAllMetadata();
						}

						ImGui::EndTable();
					}	
					UI::EndTreeNode();
				}		
			}
			ImGui::End();
		}

		void AssetManagerViewPanel::displayAllMetadata()
		{
			for (const auto& [handle, metadata] : AssetManager::s_Registry)
			{
				std::string handle = metadata.Handle.ToString();
				std::string filePath = metadata.FilePath.string();
				std::string assetType = Utils::AssetTypeToString(metadata.Type);

				UI::TableRow("Metadata",
					[]() { ImGui::Text("Handle"); },
					[&]() { 
					UI::ScopedStyleStack style(true, ImGuiStyleVar_Alpha, 0.6f);
					UI::ScopedTableColumnAutoWidth scoped(1);
					ImGui::InputText("##Handle", (char*)handle.c_str(), handle.size(), ImGuiInputTextFlags_ReadOnly); });

				UI::TableRow("Metadata",
					[]() { ImGui::Text("File Path"); },
					[&]() { 
					UI::ScopedStyleStack style(true, ImGuiStyleVar_Alpha, 0.6f);
					UI::ScopedTableColumnAutoWidth scoped(1);
					ImGui::InputText("##FilePath", (char*)filePath.c_str(), handle.size(), ImGuiInputTextFlags_ReadOnly); });


				UI::TableRow("Metadata",
					[]() { ImGui::Text("Type"); },
					[&]() { 
					UI::ScopedStyleStack style(true, ImGuiStyleVar_Alpha, 0.6f);
					UI::ScopedTableColumnAutoWidth scoped(1);
					ImGui::InputText("##Type", (char*)assetType.c_str(), assetType.size(), ImGuiInputTextFlags_ReadOnly); });
				
				ImGui::NewLine();
				ImGui::NewLine();
			}
		}

		void AssetManagerViewPanel::displaySearchedMetadata(const std::string& searchString)
		{
			for (const auto &[handle, metadata] : AssetManager::s_Registry)
			{
				std::string handle = metadata.Handle.ToString();
				std::string filePath = metadata.FilePath.string();
				std::string assetType = Utils::AssetTypeToString(metadata.Type);
				if (Utils::ToLowerCopy(handle).find(searchString) != std::string::npos
					|| Utils::ToLowerCopy(filePath).find(searchString) != std::string::npos
					|| Utils::ToLowerCopy(assetType).find(searchString) != std::string::npos)
				{
					UI::TableRow("Metadata",
						[]() { ImGui::Text("Handle"); },
						[&]() {
						UI::ScopedStyleStack style(true, ImGuiStyleVar_Alpha, 0.6f);
						UI::ScopedTableColumnAutoWidth scoped(1);
						ImGui::InputText("##Handle", (char*)handle.c_str(), handle.size(), ImGuiInputTextFlags_ReadOnly); });

					UI::TableRow("Metadata",
						[]() { ImGui::Text("File Path"); },
						[&]() {
						UI::ScopedStyleStack style(true, ImGuiStyleVar_Alpha, 0.6f);
						UI::ScopedTableColumnAutoWidth scoped(1);
						ImGui::InputText("##FilePath", (char*)filePath.c_str(), handle.size(), ImGuiInputTextFlags_ReadOnly); });


					UI::TableRow("Metadata",
						[]() { ImGui::Text("Type"); },
						[&]() {
						UI::ScopedStyleStack style(true, ImGuiStyleVar_Alpha, 0.6f);
						UI::ScopedTableColumnAutoWidth scoped(1);
						ImGui::InputText("##Type", (char*)assetType.c_str(), assetType.size(), ImGuiInputTextFlags_ReadOnly); });

					ImGui::NewLine();
					ImGui::NewLine();
				}
			}
		}

	}
}

