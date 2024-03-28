#include "stdafx.h"
#include "SpriteRendererInspector.h"

#include "XYZ/Asset/AssetManager.h"
#include "Editor/EditorHelper.h"
#include "XYZ/Scene/Components.h"

#include "XYZ/ImGui/ImGui.h"

namespace XYZ {
	namespace Editor {
		SpriteRendererInspector::SpriteRendererInspector()
			:
			Inspectable("SpriteRendererInspector"),
			m_DialogOpen(false)
		{
		}
		bool SpriteRendererInspector::OnEditorRender()
		{
			const bool result = EditorHelper::DrawComponent<SpriteRenderer>("Sprite Renderer", m_Context, [&](auto& component) {

				UI::ScopedStyleStack style(true, ImGuiStyleVar_ItemSpacing, ImVec2{ 0.0f, 5.0f });
				UI::ScopedColorStack color(true,
					ImGuiCol_Button, ImVec4{ 0.5f, 0.5f, 0.5f, 1.0f },
					ImGuiCol_ButtonHovered, ImVec4{ 0.6f, 0.6f, 0.6f, 1.0f },
					ImGuiCol_ButtonActive, ImVec4{ 0.65f, 0.65f, 0.65f, 1.0f }
				);
				const float lineHeight = GImGui->Font->FontSize + GImGui->Style.FramePadding.y * 2.0f;


				if (ImGui::BeginTable("##SpriteRendererTable", 2, ImGuiTableFlags_SizingStretchProp))
				{
					UI::TableRow("Color",
						[]() { ImGui::Text("Color"); },
						[&]() { UI::ScopedTableColumnAutoWidth scoped(4, lineHeight);
					UI::Vec4Control({ "R", "G", "B", "A" }, component.Color, 1.0f); }
					);

					std::string materialName = "None";
					if (component.Material.Valid() && AssetManager::Exist(component.Material->GetHandle()))
					{
						materialName = Utils::GetFilename(AssetManager::GetMetadata(component.Material.Value()).FilePath.string());
					}
					UI::TableRow("Material",
						[]() { ImGui::Text("Material"); },
						[&]() { 
							UI::ScopedWidth w(150.0f);					    
							ImGui::InputText("##Material", (char*)materialName.c_str(), materialName.size(), ImGuiInputTextFlags_ReadOnly); 
							
							EditorHelper::AssetDragAcceptor(component.Material.Value());
						}
					);
			

					std::string subTextureName = "None";
					if (component.SubTexture.Valid() && AssetManager::Exist(component.SubTexture->GetHandle()))
					{
						subTextureName = Utils::GetFilename(AssetManager::GetMetadata(component.SubTexture.Value()).FilePath.string());
					}
					UI::TableRow("SubTexture",
						[]() { ImGui::Text("SubTexture"); },
						[&]() { 
							UI::ScopedWidth w(150.0f);
							ImGui::InputText("##SubTexture", (char*)subTextureName.c_str(), subTextureName.size(), ImGuiInputTextFlags_ReadOnly); 
							EditorHelper::AssetDragAcceptor(component.SubTexture.Value());
						}
					);

					UI::TableRow("SortLayer",
						[]() { ImGui::Text("Sort Layer"); },
						[&]() { UI::ScopedWidth w(100.0f); ImGui::InputInt("##SortLayer", (int*)&component.SortLayer); }
					);

					UI::TableRow("Visible",
						[]() { ImGui::Text("Visible"); },
						[&]() { ImGui::Checkbox("##Visible", &component.Visible); }
					);
					ImGui::EndTable();
				}
			});

			if (m_Dialog && m_DialogOpen)
				m_Dialog();

			return result;
		}

		void SpriteRendererInspector::SetSceneEntity(const SceneEntity& entity)
		{
			m_Context = entity;
		}

		void SpriteRendererInspector::selectSubTextureDialog()
		{
			const int flags = ImGuiWindowFlags_NoDocking;
			if (ImGui::Begin("Select SubTexture", &m_DialogOpen, flags))
			{
				//const auto subTextures = std::move(AssetManager::FindAssetsByType(AssetType::SubTexture));
				//for (const Ref<SubTexture>& subTexture : subTextures)
				//{
				//	if (subTexture->IsLoaded)
				//	{
				//		const Ref<Texture>& texture = subTexture->GetTexture();
				//		const glm::vec4& texCoords = subTexture->GetTexCoords();
				//		ImGui::PushID(subTexture->FileName.c_str());
				//		if (ImGui::ImageButton((void*)(uint64_t)texture.Raw(), { 50.0f, 50.0f }, { texCoords.x, texCoords.w }, { texCoords.z, texCoords.y }))
				//		{
				//			m_Context.GetComponent<SpriteRenderer>().SubTexture = subTexture;
				//			m_DialogOpen = false;
				//			ImGui::PopID();
				//			break;
				//		}
				//		ImGui::PopID();
				//	}
				//}
			}
			ImGui::End();
		}
		void SpriteRendererInspector::selectMaterialDialog()
		{
			const int flags = ImGuiWindowFlags_NoDocking;
			if (ImGui::Begin("Select Material", &m_DialogOpen, flags))
			{
				//const auto materials = std::move(AssetManager::FindAssetsByType(AssetType::Material));
				//for (const Ref<Material>& material : materials)
				//{
				//	if (material->IsLoaded)
				//	{
				//		if (ImGui::Button(material->FileName.c_str()))
				//		{
				//			m_Context.GetComponent<SpriteRenderer>().Material = material;
				//			m_DialogOpen = false;
				//			break;
				//		}
				//	}
				//}
			}
			ImGui::End();
		}
	}
}