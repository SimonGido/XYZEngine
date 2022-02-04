#include "stdafx.h"
#include "SpriteRendererInspector.h"

#include "XYZ/Asset/AssetManager.h"
#include "Editor/EditorHelper.h"
#include "XYZ/Scene/Components.h"

#include "XYZ/ImGui/ImGui.h"

namespace XYZ {

	SpriteRendererInspector::SpriteRendererInspector()
		:
		Inspectable("SpriteRendererInspector"),
		m_DialogOpen(false)
	{
	}
	bool SpriteRendererInspector::OnEditorRender()
	{
		const bool result = EditorHelper::DrawComponent<SpriteRenderer>("Sprite Renderer", m_Context, [&](auto& component) {

			UI::ScopedStyleStack style(ImGuiStyleVar_ItemSpacing, ImVec2{ 0.0f, 5.0f });
			UI::ScopedColorStack color(
				ImGuiCol_Button, ImVec4{ 0.5f, 0.5f, 0.5f, 1.0f },
				ImGuiCol_ButtonHovered, ImVec4{ 0.6f, 0.6f, 0.6f, 1.0f },
				ImGuiCol_ButtonActive, ImVec4{ 0.65f, 0.65f, 0.65f, 1.0f }
			);
			const float lineHeight = GImGui->Font->FontSize + GImGui->Style.FramePadding.y * 2.0f;


			if (ImGui::BeginTable("##SpriteRendererTable", 2, ImGuiTableFlags_SizingFixedFit))
			{
				UI::TableRow("Color",
					[]() { ImGui::Text("Color"); },
					[&]() { UI::ScopedTableColumnAutoWidth scoped(4, lineHeight);
							UI::Vec4Control({ "R", "G", "B", "A"}, component.Color, 1.0f); }
				);

				//UI::Utils::SetPathBuffer(component.Material->FileName);
				//UI::TableRow("Material",
				//	[]() { ImGui::Text("Material"); },
				//	[&]() { UI::ScopedWidth w(100.0f);
				//		    ImGui::InputText("##Material", UI::Utils::GetPathBuffer(), _MAX_PATH); }
				//);
				//
				//UI::Utils::SetPathBuffer(component.SubTexture->FileName);
				//UI::TableRow("SubTexture",
				//	[]() { ImGui::Text("SubTexture"); },
				//	[&]() { UI::ScopedWidth w(100.0f);
				//			ImGui::InputText("##SubTexture", UI::Utils::GetPathBuffer(), _MAX_PATH); }
				//);

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
			/*
			EditorHelper::DrawColorControl("Color", component.Color);
			// Material
			{
				EditorHelper::BeginColumns("Material");

				ImGui::PushItemWidth(150.0f);

				char materialPath[_MAX_PATH];
				memcpy(materialPath, component.Material->FileName.c_str(), component.Material->FileName.size());
				materialPath[component.Material->FileName.size()] = '\0';

				ImGui::InputText("##Material", materialPath, _MAX_PATH);
				if (ImGui::IsItemClicked(ImGuiMouseButton_Left))
				{
					m_Dialog = Hook(&SpriteRendererInspector::selectMaterialDialog, this);
					m_DialogOpen = true;
				}
				ImGui::PopItemWidth();

				EditorHelper::EndColumns();
			}
			/////////////////
			// SubTexture
			{
				EditorHelper::BeginColumns("SubTexture");
				ImGui::PushItemWidth(150.0f);

				char subTexturePath[_MAX_PATH];
				memcpy(subTexturePath, component.SubTexture->FileName.c_str(), component.SubTexture->FileName.size());
				subTexturePath[component.SubTexture->FileName.size()] = '\0';

				ImGui::InputText("##SubTexture", subTexturePath, _MAX_PATH);
				if (ImGui::IsItemClicked(ImGuiMouseButton_Left))
				{
					m_Dialog = Hook(&SpriteRendererInspector::selectSubTextureDialog, this);
					m_DialogOpen = true;
				}

				ImGui::PopItemWidth();
				EditorHelper::EndColumns();
			}
			/////////////////
			// Sort Layer
			{
				EditorHelper::BeginColumns("Sort Layer");

				ImGui::PushItemWidth(75.0f);
				ImGui::InputInt("##Sort", (int*)&component.SortLayer);
				ImGui::PopItemWidth();

				EditorHelper::EndColumns();
			}
			/////////////////
			// Visible
			{
				EditorHelper::BeginColumns("Visible");
				ImGui::Checkbox("##Visible", &component.Visible);
				EditorHelper::EndColumns();
			}
			/////////////////
			*/
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