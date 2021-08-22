#include "stdafx.h"
#include "AssetBrowser.h"

#include "XYZ/Utils/StringUtils.h"
#include "XYZ/Asset/AssetManager.h"
#include "XYZ/Scene/Scene.h"
#include "XYZ/Scene/SceneSerializer.h"
#include "XYZ/Renderer/Font.h"
#include "XYZ/Animation/Animation.h"
#include "XYZ/Utils/FileSystem.h"

#include <imgui.h>


namespace XYZ {
	namespace Editor {

		static glm::vec4 CalculateTexCoords(const glm::vec2& coords, const glm::vec2& size, const glm::vec2& textureSize)
		{
			return {
				(coords.x * size.x) / textureSize.x,
				((coords.y + 1) * size.y) / textureSize.y,
				((coords.x + 1) * size.x) / textureSize.x,
				(coords.y * size.y) / textureSize.y,
			};
		}
		static std::filesystem::path s_AssetPath = "Assets";

		AssetBrowser::AssetBrowser()
			:
			m_IconSize(50.0f),
			m_ArrowSize(25.0f),
			m_CurrentDirectory(s_AssetPath),
			m_ViewportHovered(false),
			m_ViewportFocused(false)
		{
			m_Texture = Texture2D::Create({}, "Assets/Textures/Gui/icons.png");
			float divisor = 4.0f;
			float width  = (float)m_Texture->GetWidth();
			float height = (float)m_Texture->GetHeight();
			glm::vec2 size = glm::vec2(width / divisor, height / divisor);


			m_TexCoords[Arrow]     = CalculateTexCoords(glm::vec2(0, 2), size, { width, height });
			m_TexCoords[Folder]    = CalculateTexCoords(glm::vec2(0, 3), size, { width, height });
			m_TexCoords[Shader]    = CalculateTexCoords(glm::vec2(1, 3), size, { width, height });
			m_TexCoords[Script]    = CalculateTexCoords(glm::vec2(3, 3), size, { width, height });
			m_TexCoords[Material]  = CalculateTexCoords(glm::vec2(2, 3), size, { width, height });
			m_TexCoords[Texture]   = CalculateTexCoords(glm::vec2(2, 2), size, { width, height });
			m_TexCoords[Mesh]	   = CalculateTexCoords(glm::vec2(1, 2), size, { width, height });
			m_TexCoords[Scene]	   = CalculateTexCoords(glm::vec2(1, 1), size, { width, height });
			m_TexCoords[Animation] = CalculateTexCoords(glm::vec2(0, 1), size, { width, height });
			m_Colors[ArrowColor]	    = glm::vec4(0.5f, 0.5f, 0.5f, 1.0f);
			m_Colors[ArrowInvalidColor] = glm::vec4(0.3f);
		}
		void AssetBrowser::SetPath(const std::string& path)
		{
			m_CurrentDirectory = path;
		}
		void AssetBrowser::OnImGuiRender()
		{
			if (ImGui::Begin("Asset Browser"))
			{
				m_ViewportFocused = ImGui::IsWindowFocused();
				m_ViewportHovered = ImGui::IsWindowHovered();
				if (ImGui::GetIO().MouseDown[ImGuiMouseButton_Left]
					&& m_ViewportFocused
					&& m_ViewportHovered)
				{
					m_SelectedFile.clear();
					if (m_Callback)
						m_Callback(GetSelectedAsset());
				}

				bool backArrowAvailable = m_CurrentDirectory != std::filesystem::path(s_AssetPath);
				bool frontArrowAvailable = !m_DirectoriesVisited.empty();

				ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
				glm::vec4 arrowColor = backArrowAvailable ? m_Colors[ArrowColor] : m_Colors[ArrowInvalidColor];
				ImGui::PushID("BackArrow");
				if (ImGui::ImageButton((void*)(uint64_t)m_Texture->GetRendererID(), { m_ArrowSize.x, m_ArrowSize.y }, { m_TexCoords[Arrow].z, m_TexCoords[Arrow].y }, { m_TexCoords[Arrow].x, m_TexCoords[Arrow].w }, -1, {}, { arrowColor.r, arrowColor.g, arrowColor.b, arrowColor.a })
				 && backArrowAvailable)
				{
					m_DirectoriesVisited.push_front(m_CurrentDirectory);
					m_CurrentDirectory = m_CurrentDirectory.parent_path();
				}
				ImGui::PopID();
				ImGui::SameLine();
				
				arrowColor = frontArrowAvailable ? m_Colors[ArrowColor] : m_Colors[ArrowInvalidColor];
				ImGui::PushID("FrontArrow");
				if (ImGui::ImageButton((void*)(uint64_t)m_Texture->GetRendererID(), { m_ArrowSize.x, m_ArrowSize.y }, { m_TexCoords[Arrow].x, m_TexCoords[Arrow].y }, { m_TexCoords[Arrow].z, m_TexCoords[Arrow].w }, -1, {}, { arrowColor.r, arrowColor.g, arrowColor.b, arrowColor.a })
				&& frontArrowAvailable)
				{
					m_CurrentDirectory = m_DirectoriesVisited.front();
					m_DirectoriesVisited.pop_front();
				}
				ImGui::PopID();
				ImGui::SameLine();
			
				char tempPathBuffer[_MAX_PATH];
				size_t length = m_CurrentDirectory.string().size();
				memcpy(tempPathBuffer, m_CurrentDirectory.string().c_str(), length);
				tempPathBuffer[length] = '\0';

				if (ImGui::InputText("###", tempPathBuffer, _MAX_PATH))
				{
					m_CurrentDirectory = tempPathBuffer;
					m_DirectoriesVisited.clear();
				}

				createAsset();
				processDirectory();
				ImGui::PopStyleColor();
			}
			ImGui::End();
		}
		Ref<Asset> AssetBrowser::GetSelectedAsset() const
		{
			if (!m_SelectedFile.empty())
			{
				std::string fullFilePath = m_CurrentDirectory.string() + "/" + m_SelectedFile.string();
				std::replace(fullFilePath.begin(), fullFilePath.end(), '\\', '/');
				AssetType type = AssetManager::GetAssetTypeFromExtension(Utils::GetExtension(m_SelectedFile.string()));
				auto assetHandle = AssetManager::GetAssetHandle(fullFilePath);
				switch (type)
				{
				case XYZ::AssetType::Scene:
					return AssetManager::GetAsset<XYZ::Scene>(assetHandle);
					break;
				case XYZ::AssetType::Texture:
					return AssetManager::GetAsset<XYZ::Texture>(assetHandle);
					break;
				case XYZ::AssetType::SubTexture:
					return AssetManager::GetAsset<XYZ::SubTexture>(assetHandle);
					break;
				case XYZ::AssetType::Material:
					return AssetManager::GetAsset<XYZ::Material>(assetHandle);
					break;
				case XYZ::AssetType::Shader:
					return AssetManager::GetAsset<XYZ::Shader>(assetHandle);
					break;
				case XYZ::AssetType::Font:
					return AssetManager::GetAsset<XYZ::Font>(assetHandle);
					break;
				case XYZ::AssetType::Audio:				
					break;
				case XYZ::AssetType::Script:		
					break;
				case XYZ::AssetType::SkeletalMesh:
					break;
				case XYZ::AssetType::None:
					break;
				default:
					break;
				}
			}
			return Ref<Asset>();
		}
		void AssetBrowser::createAsset()
		{
			if (ImGui::IsMouseDown(ImGuiMouseButton_Right) && ImGui::IsWindowHovered())
			{
				ImGui::OpenPopup("CreateAsset");
			}
			if (ImGui::BeginPopup("CreateAsset"))
			{
				if (ImGui::MenuItem("Create Folder"))
				{
					std::string fullpath = getUniqueAssetName("New Folder", nullptr);
					FileSystem::CreateFolder(fullpath);
					AssetManager::CreateDirectory(fullpath);
				}
				if (ImGui::MenuItem("Create Scene"))
				{
					std::string fullpath = getUniqueAssetName("New Scene", ".xyz");
					Ref<XYZ::Scene> scene = Ref<XYZ::Scene>::Create("");
					scene->FilePath = fullpath;
					scene->Type = AssetType::Scene;
					AssetSerializer::SerializeAsset(scene);
					ImGui::CloseCurrentPopup();
				}
				else if (ImGui::MenuItem("Create Animation"))
				{
					std::string fullpath = getUniqueAssetName("New Animation", ".anim");
					Ref<XYZ::Animation> animation = Ref<XYZ::Animation>::Create(SceneEntity());
					animation->FilePath = fullpath;
					animation->Type = AssetType::Animation;
					AssetSerializer::SerializeAsset(animation);
					ImGui::CloseCurrentPopup();
				}
				ImGui::EndPopup();
			}
		}
		void AssetBrowser::processDirectory()
		{
			if (!std::filesystem::is_directory(m_CurrentDirectory))
				return;

			static float padding = 32.0f;
			float cellSize = m_IconSize.x + padding;

			float panelWidth = ImGui::GetContentRegionAvail().x;
			int columnCount = (int)(panelWidth / cellSize);
			if (columnCount < 1)
				columnCount = 1;

			ImGui::Columns(columnCount, 0, false);
			for (auto& it : std::filesystem::directory_iterator(m_CurrentDirectory))
			{				
				std::string name = it.path().filename().string();
				ImGui::PushID(name.c_str());
				
				if (it.is_directory())
				{
					if (ImGui::ImageButton((void*)(uint64_t)m_Texture->GetRendererID(), { m_IconSize.x, m_IconSize.y }, { m_TexCoords[Folder].x, m_TexCoords[Folder].y }, { m_TexCoords[Folder].z, m_TexCoords[Folder].w }))
					{
						m_CurrentDirectory /= it.path().filename();
						m_DirectoriesVisited.clear();
					}
				}
				else
				{
					AssetType type = AssetManager::GetAssetTypeFromExtension(Utils::GetExtension(name));
					if (type == AssetType::None)
					{
						ImGui::PopID();
						continue;
					}
					size_t index = assetTypeToTexCoordsIndex(type);
					if (ImGui::ImageButton((void*)(uint64_t)m_Texture->GetRendererID(), { m_IconSize.x, m_IconSize.y }, { m_TexCoords[index].x, m_TexCoords[index].y }, { m_TexCoords[index].z, m_TexCoords[index].w }))
					{
						m_SelectedFile = name;
						if (m_Callback)
							m_Callback(GetSelectedAsset());
					}

					dragAndDrop(std::filesystem::relative(it, s_AssetPath));

				}

				ImGui::TextWrapped(name.c_str());
				ImGui::NextColumn();
				ImGui::PopID();
			}
			ImGui::Columns(1);
		}
		size_t AssetBrowser::assetTypeToTexCoordsIndex(AssetType type) const
		{
			switch (type)
			{
			case XYZ::AssetType::Scene:
				return Scene;
				break;
			case XYZ::AssetType::Texture:
				return Texture;
				break;
			case XYZ::AssetType::SubTexture:
				return SubTexture;
				break;
			case XYZ::AssetType::Material:
				return Material;
				break;
			case XYZ::AssetType::Shader:
				return Shader;
				break;
			case XYZ::AssetType::Font:
				return Font;
				break;
			case XYZ::AssetType::Audio:
				return Audio;
				break;
			case XYZ::AssetType::Script:
				return Script;
				break;
			case XYZ::AssetType::SkeletalMesh:
				return Mesh;
				break;
			case XYZ::AssetType::Animation:
				return Animation;
				break;
			case XYZ::AssetType::None:
				return NumTypes;
				break;
			default:
				break;
			}
			return NumTypes;
		}
		void AssetBrowser::dragAndDrop(const std::filesystem::path& path)
		{
			if (ImGui::BeginDragDropSource())
			{
				std::string str = path.string();
				const char* itemPath = str.c_str();
				ImGui::SetDragDropPayload("ASSET_BROWSER_ITEM", itemPath, str.size(), ImGuiCond_Once);
				ImGui::EndDragDropSource();
			}
		}
		std::string AssetBrowser::getUniqueAssetName(const char* fileName, const char* extension) const
		{
			char fileNameTmp[60];
			std::string fullpath = m_CurrentDirectory.string() + "/" + fileName;
			if (extension)
				fullpath += extension;
	
			if (std::filesystem::exists(fullpath))
			{
				uint32_t index = 0;
				if (extension)
					sprintf(fileNameTmp, "%s%d%s", fileName, index, extension);
				else
					sprintf(fileNameTmp, "%s%d", fileName, index);

				fullpath = m_CurrentDirectory.string() + "/" + fileNameTmp;
				while (std::filesystem::exists(fullpath))
				{
					if (extension)
						sprintf(fileNameTmp, "%s%d%s", fileName, index, extension);
					else
						sprintf(fileNameTmp, "%s%d", fileName, index);
					fullpath = m_CurrentDirectory.string() + "/" + fileNameTmp;
				}
			}
			return fullpath;
		}
	}
}