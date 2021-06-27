#include "stdafx.h"
#include "SpriteRendererInspector.h"

#include "XYZ/Asset/AssetManager.h"
#include "XYZ/Editor/EditorHelper.h"
#include "XYZ/Scene/Components.h"

namespace XYZ {
	SpriteRendererInspector::SpriteRendererInspector()
		:
		m_DialogOpen(false)
	{
	}
	bool SpriteRendererInspector::OnEditorRender()
	{
		return EditorHelper::DrawComponent<SpriteRenderer>("Sprite Renderer", m_Context, [&](auto& component) {

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
		});

		if (m_Dialog && m_DialogOpen)
			m_Dialog();
	}

	void SpriteRendererInspector::selectSubTextureDialog()
	{
		int flags = ImGuiWindowFlags_NoDocking;
		if (ImGui::Begin("Select SubTexture", &m_DialogOpen, flags))
		{
			auto subTextures = std::move(AssetManager::FindAssetsByType(AssetType::SubTexture));
			for (const Ref<SubTexture>& subTexture : subTextures)
			{
				if (subTexture->IsLoaded)
				{
					const Ref<Texture>& texture = subTexture->GetTexture();
					const glm::vec4& texCoords = subTexture->GetTexCoords();
					if (ImGui::ImageButton((void*)(uint64_t)texture->GetRendererID(), { 50.0f, 50.0f }, { texCoords.x, texCoords.w }, { texCoords.z, texCoords.y }))
					{
						m_Context.GetComponent<SpriteRenderer>().SubTexture = subTexture;
						m_DialogOpen = false;
						break;
					}
				}
			}
		}
		ImGui::End();
	}
	void SpriteRendererInspector::selectMaterialDialog()
	{
		int flags = ImGuiWindowFlags_NoDocking;
		if (ImGui::Begin("Select Material", &m_DialogOpen, flags))
		{
			auto materials = std::move(AssetManager::FindAssetsByType(AssetType::Material));
			for (const Ref<Material>& material : materials)
			{
				if (material->IsLoaded)
				{
					if (ImGui::Button(material->FileName.c_str()))
					{
						m_Context.GetComponent<SpriteRenderer>().Material = material;
						m_DialogOpen = false;
						break;
					}
				}
			}
		}
		ImGui::End();
	}
}