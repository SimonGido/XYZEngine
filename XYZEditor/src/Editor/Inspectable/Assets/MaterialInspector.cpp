#include "stdafx.h"
#include "MaterialInspector.h"

#include "XYZ/Asset/AssetManager.h"

#include "XYZ/ImGui/ImGui.h"

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
			ImGui::Text("Shader");
			handleShader();

			ImGui::Text("Textures");
			handleTextures();

			ImGui::Text("Texture Arrays");
			handleTextureArrays();

			return false;
		}
		void MaterialInspector::SetAsset(const Ref<Asset>& asset)
		{
			m_MaterialAsset = asset.As<MaterialAsset>();
		}
		void MaterialInspector::handleShader()
		{
			const std::string& name = m_MaterialAsset->GetShader()->GetName();
			ImGui::InputText("##ShaderName", (char*)name.c_str(), name.size(), ImGuiInputTextFlags_ReadOnly);

			char* shaderAssetPath = nullptr;
			if (UI::DragDropTarget("AssetDragAndDrop", &shaderAssetPath))
			{
				std::filesystem::path path(shaderAssetPath);
				if (AssetManager::Exist(path))
				{
					auto& metadata = AssetManager::GetMetadata(path);
					if (metadata.Type == AssetType::Shader)
					{
						Ref<ShaderAsset> shaderAsset = AssetManager::GetAsset<ShaderAsset>(metadata.Handle);
						m_MaterialAsset->SetShaderAsset(shaderAsset);
						AssetManager::Serialize(m_MaterialAsset->GetHandle());
					}
				}
			}

			if (ImGui::IsItemClicked(ImGuiMouseButton_Left))
				ImGui::OpenPopup("Shader");

			if (ImGui::BeginPopup("Shader"))
			{
				std::vector<AssetMetadata> shaderAssetsMeta = AssetManager::FindAllMetadata(AssetType::Shader);
				for (auto& shaderMeta : shaderAssetsMeta)
				{
					auto name = shaderMeta.FilePath.filename().string();
					if (ImGui::MenuItem(name.c_str()))
					{
						Ref<ShaderAsset> shaderAsset = AssetManager::GetAsset<ShaderAsset>(shaderMeta.Handle);
						m_MaterialAsset->SetShaderAsset(shaderAsset);
						AssetManager::Serialize(m_MaterialAsset->GetHandle());
						break;
					}
				}
				ImGui::EndPopup();
			}
		}

		void MaterialInspector::handleTextures()
		{
			auto& textures = m_MaterialAsset->GetTextures();
			for (auto& textureData : textures)
			{
				std::string name = "None";
				if (textureData.Texture.Raw())
					name = Utils::GetFilenameWithoutExtension(textureData.Texture->GetPath());
				

				UI::ScopedID id(name.c_str());
				ImGui::InputText("##TextureName", (char*)name.c_str(), name.size(), ImGuiInputTextFlags_ReadOnly);
				
				char* textureAssetPath = nullptr;
				if (UI::DragDropTarget("AssetDragAndDrop", &textureAssetPath))
				{
					std::filesystem::path path(textureAssetPath);
					if (AssetManager::Exist(path))
					{
						auto& metadata = AssetManager::GetMetadata(path);
						if (metadata.Type == AssetType::Texture)
						{
							Ref<Texture2D> textureAsset = AssetManager::GetAsset<Texture2D>(metadata.Handle);
							m_MaterialAsset->SetTexture(textureData.Name, textureAsset);
							AssetManager::Serialize(m_MaterialAsset->GetHandle());
						}
					}
				}
			}
		}

		void MaterialInspector::handleTextureArrays()
		{
			auto& textureArrays = m_MaterialAsset->GetTextureArrays();
			for (auto& textureArrayData : textureArrays)
			{
				uint32_t index = 0;
				for (auto& texture : textureArrayData.Textures)
				{
					std::string textureArrayName = textureArrayData.Name + std::to_string(index);
					UI::ScopedID id(textureArrayName.c_str());

					std::string name = "None";
					if (texture.Raw())
						name = Utils::GetFilenameWithoutExtension(texture->GetPath());

					ImGui::Text(textureArrayName.c_str());
					ImGui::SameLine();
					ImGui::InputText("##TextureName", (char*)name.c_str(), name.size(), ImGuiInputTextFlags_ReadOnly);

					char* textureAssetPath = nullptr;
					if (UI::DragDropTarget("AssetDragAndDrop", &textureAssetPath))
					{
						std::filesystem::path path(textureAssetPath);
						if (AssetManager::Exist(path))
						{
							auto& metadata = AssetManager::GetMetadata(path);
							if (metadata.Type == AssetType::Texture)
							{
								Ref<Texture2D> textureAsset = AssetManager::GetAsset<Texture2D>(metadata.Handle);
								m_MaterialAsset->SetTexture(textureArrayData.Name, textureAsset, index);
								AssetManager::Serialize(m_MaterialAsset->GetHandle());
							}
						}
					}
					index++;
				}
			}
		}

	}
}