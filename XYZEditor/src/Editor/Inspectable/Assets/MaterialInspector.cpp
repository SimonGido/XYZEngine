#include "stdafx.h"
#include "MaterialInspector.h"

#include "XYZ/Asset/AssetManager.h"

#include <imgui/imgui.h>

#include <glm/gtc/type_ptr.hpp>


namespace XYZ {
	namespace Editor {
		MaterialInspector::MaterialInspector()
			:
			Inspectable("MaterialInspector")
		{
		}
		bool MaterialInspector::OnEditorRender()
		{		
			std::string name = m_MaterialAsset->GetShader()->GetName();
			ImGui::InputText("##ShaderName", (char*)name.c_str(), name.size(), ImGuiInputTextFlags_ReadOnly);
			if (ImGui::IsItemClicked(ImGuiMouseButton_Left))
				ImGui::OpenPopup("Shader");

			if (ImGui::BeginPopup("Shader"))
			{
				std::vector<Ref<ShaderAsset>> shaderAssets = AssetManager::FindAllAssets<ShaderAsset>(AssetType::Shader);
				for (auto& shaderAsset : shaderAssets)
				{
					if (ImGui::MenuItem(shaderAsset->GetShader()->GetName().c_str()))
					{
						m_MaterialAsset->SetShaderAsset(shaderAsset);
						break;
					}
				}
				ImGui::EndPopup();
			}

			return false;
		}
		void MaterialInspector::SetAsset(const Ref<Asset>& asset)
		{
			m_MaterialAsset = asset.As<MaterialAsset>();
		}
	}
}