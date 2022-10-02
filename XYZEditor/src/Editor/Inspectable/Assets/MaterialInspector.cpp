#include "stdafx.h"
#include "MaterialInspector.h"

#include "XYZ/Asset/AssetManager.h"

#include "XYZ/ImGui/ImGui.h"

#include "Editor/EditorHelper.h"

#include <imgui/imgui.h>

#include <glm/gtc/type_ptr.hpp>


namespace XYZ {
	namespace Editor {
		static bool ValueControl(const char* stringID, bool& value)
		{
			std::string valueID = "##";
			valueID += stringID;
			bool result = false;

			UI::TableRow(stringID,
				[&]() { ImGui::Text(stringID); },
				[&]() { result = ImGui::Checkbox(valueID.c_str(), &value); });

			return result;
		}
		static bool ValueControl(const char* stringID, float& value)
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
		static bool ValueControl(const char* stringID, int& value)
		{
			std::string valueID = "##";
			valueID += stringID;
			bool result = false;

			UI::TableRow(stringID,
				[&]() { ImGui::Text(stringID); },
				[&]() { result = ImGui::DragInt(valueID.c_str(), &value, 0.05f); });

			return result;
		}
		static bool ValueControl(const char* stringID, uint32_t& value)
		{
			std::string valueID = "##";
			valueID += stringID;
			bool result = false;

			UI::TableRow(stringID,
				[&]() { ImGui::Text(stringID); },
				[&]() { result = ImGui::DragInt(valueID.c_str(), (int*)&value, 0.05f); });

			return result;
		}
		static bool ValueControl(const char* stringID, glm::vec2& value)
		{
			std::string valueID = "##";
			valueID += stringID;
			bool result = false;

			UI::TableRow(stringID,
				[&]() { ImGui::Text(stringID); },
				[&]() { result = ImGui::DragFloat2(valueID.c_str(), (float*)&value, 0.05f); });


			return result;
		}
		static bool ValueControl(const char* stringID, glm::vec3& value)
		{
			std::string valueID = "##";
			valueID += stringID;
			bool result = false;

			UI::TableRow(stringID,
				[&]() { ImGui::Text(stringID); },
				[&]() { result = ImGui::DragFloat3(valueID.c_str(), (float*)&value, 0.05f); });

			return result;
		}
		static bool ValueControl(const char* stringID, glm::vec4& value)
		{
			std::string valueID = "##";
			valueID += stringID;
			bool result = false;

			UI::TableRow(stringID,
				[&]() { ImGui::Text(stringID); },
				[&]() { result = ImGui::DragFloat4(valueID.c_str(), (float*)&value, 0.05f); });

			return result;
		}

		MaterialInspector::MaterialInspector()
			:
			Inspectable("MaterialInspector")
		{
		}
		bool MaterialInspector::OnEditorRender()
		{	
			// It is reloaded each time we serialize it, ( update reference )
			m_MaterialAsset = AssetManager::GetAsset<MaterialAsset>(m_MaterialAsset->GetHandle());

			ImGui::Text("Shader");
			handleShader();
			
			if (!m_MaterialAsset->GetTextures().empty())
			{
				ImGui::Text("Textures");
				handleTextures();
			}
			if (!m_MaterialAsset->GetTextureArrays().empty())
			{
				ImGui::Text("Texture Arrays");
				handleTextureArrays();
			}
			handleUniforms();

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
			
			Ref<ShaderAsset> shaderAsset;
			if (EditorHelper::AssetDragAcceptor(shaderAsset))
			{
				m_MaterialAsset->SetShaderAsset(shaderAsset);
				AssetManager::Serialize(m_MaterialAsset->GetHandle());
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

			bool opaque = m_MaterialAsset->IsOpaque();
			if (ImGui::Checkbox("Opaque", &opaque))
			{
				m_MaterialAsset->SetOpaque(opaque);
				AssetManager::Serialize(m_MaterialAsset->GetHandle());
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
				

				Ref<Texture2D> textureAsset;
				if (EditorHelper::AssetDragAcceptor(textureAsset))
				{
					m_MaterialAsset->SetTexture(textureData.Name, textureAsset);
					AssetManager::Serialize(m_MaterialAsset->GetHandle());
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

					Ref<Texture2D> textureAsset;
					if (EditorHelper::AssetDragAcceptor(textureAsset))
					{
						m_MaterialAsset->SetTexture(textureArrayData.Name, textureAsset, index);
						AssetManager::Serialize(m_MaterialAsset->GetHandle());
					}
					index++;
				}
			}
		}

		void MaterialInspector::handleUniforms()
		{
			bool modified = false;
			if (ImGui::BeginTable("MaterialValues", 2, ImGuiTableFlags_SizingStretchProp))
			{
				auto shader = m_MaterialAsset->GetShader();
				for (auto& [name, buffer] : shader->GetBuffers())
				{
					for (auto& [uniName, uni] : buffer.Uniforms)
					{
						switch (uni.GetDataType())
						{
						case ShaderUniformDataType::Int:   modified |= ValueControl(uniName.c_str(), m_MaterialAsset->Get<int>(uniName));		 break;
						case ShaderUniformDataType::UInt:  modified |= ValueControl(uniName.c_str(), m_MaterialAsset->Get<uint32_t>(uniName));   break;
						case ShaderUniformDataType::Float: modified |= ValueControl(uniName.c_str(), m_MaterialAsset->Get<float>(uniName));		 break;
						case ShaderUniformDataType::Vec2:  modified |= ValueControl(uniName.c_str(), m_MaterialAsset->Get<glm::vec2>(uniName));  break;
						case ShaderUniformDataType::Vec3:  modified |= ValueControl(uniName.c_str(), m_MaterialAsset->Get<glm::vec3>(uniName));  break;
						case ShaderUniformDataType::Vec4:  modified |= ValueControl(uniName.c_str(), m_MaterialAsset->Get<glm::vec4>(uniName));  break;
							//case ShaderUniformDataType::Mat3:  ValueControl(name.c_str(), m_MaterialAsset->Get<int>(name));
							//case ShaderUniformDataType::Mat4:  ValueControl(name.c_str(), m_MaterialAsset->Get<int>(name));
						}
					}
				}
				ImGui::EndTable();
			}
			if (modified)
			{
				AssetManager::Serialize(m_MaterialAsset->GetHandle());
			}
		}

	}
}