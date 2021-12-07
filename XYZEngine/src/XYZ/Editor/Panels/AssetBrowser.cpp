#include "stdafx.h"
#include "AssetBrowser.h"

#include "XYZ/Utils/StringUtils.h"
#include "XYZ/Asset/AssetManager.h"
#include "XYZ/Scene/Scene.h"
#include "XYZ/Scene/SceneSerializer.h"
#include "XYZ/Renderer/Font.h"
#include "XYZ/Animation/Animation.h"
#include "XYZ/Utils/FileSystem.h"
#include "XYZ/ImGui/ImGui.h"

#include <imgui.h>


namespace XYZ {
	namespace Editor {

		static std::filesystem::path s_AssetPath = "Assets";

		AssetBrowser::AssetBrowser(std::string name)
			:
			EditorPanel(std::move(name)),
			m_CurrentDirectory(s_AssetPath),
			m_ViewportHovered(false),
			m_ViewportFocused(false)
		{
			registerFileTypeExtensions();
			m_Texture = Texture2D::Create("Assets/Textures/Gui/icons.png");

			const float divisor = 4.0f;
			float width  = (float)m_Texture->GetWidth();
			float height = (float)m_Texture->GetHeight();
			const glm::vec2 cellSize = glm::vec2(width / divisor, height / divisor);
			const glm::vec2 textureSize = { width, height };


			m_IconSize = ImVec2(50.0f, 50.0f);
			m_ArrowSize = ImVec2(25.0f, 25.0f);

			
			m_TexCoords[RightArrow] = UV::Calculate(glm::vec2(0, 2), cellSize, textureSize);
			m_TexCoords[LeftArrow] = m_TexCoords[RightArrow];
			std::swap(m_TexCoords[LeftArrow].UV0.x, m_TexCoords[LeftArrow].UV1.x);
			
			m_TexCoords[Folder]		= UV::Calculate(glm::vec2(0, 3), cellSize, textureSize);
			m_TexCoords[Shader]		= UV::Calculate(glm::vec2(1, 3), cellSize, textureSize);
			m_TexCoords[Script]		= UV::Calculate(glm::vec2(3, 3), cellSize, textureSize);
			m_TexCoords[Material]	= UV::Calculate(glm::vec2(2, 3), cellSize, textureSize);
			m_TexCoords[Texture]	= UV::Calculate(glm::vec2(2, 2), cellSize, textureSize);
			m_TexCoords[Mesh]		= UV::Calculate(glm::vec2(1, 2), cellSize, textureSize);
			m_TexCoords[Scene]		= UV::Calculate(glm::vec2(1, 1), cellSize, textureSize);
			m_TexCoords[Animation]	= UV::Calculate(glm::vec2(0, 1), cellSize, textureSize);

			m_Colors[ArrowColor]	= glm::vec4(0.6f, 0.6f, 0.6f, 1.0f);
			m_Colors[IconColor]		= glm::vec4(0.6f, 0.6f, 0.6f, 1.0f);
			m_Colors[DisabledColor] = glm::vec4(0.5f, 0.5f, 0.5f, 0.3f);
			m_Colors[HoverColor]    = glm::vec4(0.8f, 0.8f, 0.8f, 1.0f);
			m_Colors[ClickColor]	= glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
		}
	
		void AssetBrowser::OnImGuiRender(bool& open)
		{
			if (ImGui::Begin("Asset Browser", &open))
			{
				m_ViewportFocused = ImGui::IsWindowFocused();
				m_ViewportHovered = ImGui::IsWindowHovered();

				const bool backArrowAvailable = m_CurrentDirectory != std::filesystem::path(s_AssetPath);
				const bool frontArrowAvailable = !m_DirectoriesVisited.empty();

				const ImVec4 backArrowColor = backArrowAvailable ? m_Colors[ArrowColor] : m_Colors[DisabledColor];
				const ImVec4 frontArrowColor = frontArrowAvailable ? m_Colors[ArrowColor] : m_Colors[DisabledColor];
		
				ImGui::PushItemFlag(ImGuiItemFlags_Disabled, !backArrowAvailable);
				const bool backArrowPressed = UI::ImageButtonTransparent("##BackArrow",m_Texture->GetImage(), m_ArrowSize, m_Colors[HoverColor], m_Colors[ClickColor], backArrowColor,
					m_TexCoords[LeftArrow].UV0, m_TexCoords[LeftArrow].UV1);
				ImGui::PopItemFlag();
				ImGui::SameLine();


				ImGui::PushItemFlag(ImGuiItemFlags_Disabled, !frontArrowAvailable);
				const bool frontArrowPressed = UI::ImageButtonTransparent("##FrontArrow", m_Texture->GetImage(), m_ArrowSize, m_Colors[HoverColor], m_Colors[ClickColor], frontArrowColor,
					m_TexCoords[RightArrow].UV0, m_TexCoords[RightArrow].UV1);
				ImGui::PopItemFlag();
				ImGui::SameLine();


				if (backArrowPressed && backArrowAvailable)
				{
					m_DirectoriesVisited.push_front(m_CurrentDirectory);
					m_CurrentDirectory = m_CurrentDirectory.parent_path();
				}
						
				if (frontArrowPressed && frontArrowAvailable)
				{
					m_CurrentDirectory = m_DirectoriesVisited.front();
					m_DirectoriesVisited.pop_front();
				}
				
			
				UI::Utils::SetPathBuffer(m_CurrentDirectory.string());
				if (ImGui::InputText("###", UI::Utils::GetPathBuffer(), _MAX_PATH))
				{
					m_CurrentDirectory = UI::Utils::GetPathBuffer();
					m_DirectoriesVisited.clear();
				}

				processDirectory();

				if (ImGui::GetIO().MouseReleased[ImGuiMouseButton_Left]
					&& m_ViewportFocused
					&& m_ViewportHovered)
				{
					m_RightClickedFile.clear();
					m_SelectedFile.clear();			
				}
			}
			ImGui::End();
		}

		void AssetBrowser::SetPath(const std::string& path)
		{
			m_CurrentDirectory = path;
		}

		Ref<Asset> AssetBrowser::GetSelectedAsset() const
		{
			if (!m_SelectedFile.empty())
			{
				std::string fullFilePath = m_CurrentDirectory.string() + "/" + m_SelectedFile.string();
				std::replace(fullFilePath.begin(), fullFilePath.end(), '\\', '/');
				//const AssetType type = AssetManager::GetAssetTypeFromExtension(Utils::GetExtension(m_SelectedFile.string()));
				//if (type == AssetType::None)
				//	return Ref<Asset>();

				//const auto assetHandle = AssetManager::GetAssetHandle(fullFilePath);			
				//switch (type)
				//{
				//case XYZ::AssetType::Scene:
				//	return AssetManager::GetAsset<XYZ::Scene>(assetHandle);
				//	break;
				//case XYZ::AssetType::Texture:
				//	return AssetManager::GetAsset<XYZ::Texture>(assetHandle);
				//	break;
				//case XYZ::AssetType::SubTexture:
				//	return AssetManager::GetAsset<XYZ::SubTexture>(assetHandle);
				//	break;
				//case XYZ::AssetType::Material:
				//	return AssetManager::GetAsset<XYZ::Material>(assetHandle);
				//	break;
				//case XYZ::AssetType::Shader:
				//	return AssetManager::GetAsset<XYZ::Shader>(assetHandle);
				//	break;
				//case XYZ::AssetType::Font:
				//	return AssetManager::GetAsset<XYZ::Font>(assetHandle);
				//	break;
				//case XYZ::AssetType::Audio:				
				//	break;
				//case XYZ::AssetType::Script:		
				//	break;
				//case XYZ::AssetType::SkeletalMesh:
				//	break;
				//case XYZ::AssetType::None:
				//	break;
				//default:
				//	break;
				//}
			}
			return Ref<Asset>();
		}
		void AssetBrowser::registerFileTypeExtensions()
		{
			m_FileTypeExtensions["xyz"]    = FileType::Scene;
			m_FileTypeExtensions["tex"]    = FileType::Texture;
			m_FileTypeExtensions["subtex"] = FileType::SubTexture;
			m_FileTypeExtensions["mat"]    = FileType::Material;
			m_FileTypeExtensions["shader"] = FileType::Shader;
			m_FileTypeExtensions["cs"]	   = FileType::Script;
			m_FileTypeExtensions["anim"]   = FileType::Animation;
			m_FileTypeExtensions["png"]    = FileType::Png;
			m_FileTypeExtensions["jpg"]    = FileType::Jpg;
		}
		void AssetBrowser::createAsset() const
		{
			if (ImGui::MenuItem("Create Folder"))
			{
				const std::string fullpath = getUniqueAssetName("New Folder", nullptr);
				FileSystem::CreateFolder(fullpath);
				//AssetManager::CreateDirectory(fullpath);
				ImGui::CloseCurrentPopup();
			}
			if (ImGui::MenuItem("Create Scene"))
			{
				const std::string fullpath = getUniqueAssetName("New Scene", ".xyz");
				Ref<XYZ::Scene> scene = Ref<XYZ::Scene>::Create("");
				//scene->FilePath = fullpath;
				//scene->Type = AssetType::Scene;
				//AssetSerializer::SerializeAsset(scene);
				ImGui::CloseCurrentPopup();
			}
			else if (ImGui::MenuItem("Create Animation"))
			{
				const std::string fullpath = getUniqueAssetName("New Animation", ".anim");
				Ref<XYZ::Animation> animation = Ref<XYZ::Animation>::Create();
				//animation->FilePath = fullpath;
				//animation->Type = AssetType::Animation;
				//AssetSerializer::SerializeAsset(animation);
				ImGui::CloseCurrentPopup();
			}
		}
		void AssetBrowser::processDirectory()
		{
			if (!std::filesystem::is_directory(m_CurrentDirectory))
				return;

			static float padding = 32.0f;
			const float cellSize = m_IconSize.x + padding;

			const float panelWidth = ImGui::GetContentRegionAvail().x;
			int columnCount = (int)(panelWidth / cellSize);
			if (columnCount < 1)
				columnCount = 1;

			ImGui::Columns(columnCount, 0, false);
			for (auto& it : std::filesystem::directory_iterator(m_CurrentDirectory))
			{				
				std::string name = it.path().filename().string();
		
				if (it.is_directory())
				{
					const bool dirPressed = UI::ImageButtonTransparent(name.c_str(), m_Texture->GetImage(), m_IconSize, m_Colors[HoverColor], m_Colors[ClickColor], m_Colors[IconColor],
						m_TexCoords[Folder].UV0, m_TexCoords[Folder].UV1);

					if (dirPressed)
					{
						m_CurrentDirectory /= it.path().filename();
						m_DirectoriesVisited.clear();
						break;
					}
				}
				else
				{
					const size_t index = extensionToTexCoordsIndex(Utils::GetExtension(name));
					if (index == FileType::NumTypes)
						continue;


					const bool iconPressed = UI::ImageButtonTransparent(name.c_str(), m_Texture->GetImage(), m_IconSize, m_Colors[HoverColor], m_Colors[ClickColor], m_Colors[IconColor],
						m_TexCoords[index].UV0, m_TexCoords[index].UV1);
										
					if (iconPressed)
					{
						m_SelectedFile = name;
					}
					if (ImGui::IsItemClicked(ImGuiMouseButton_Right))
					{
						m_RightClickedFile = name;
					}


					std::string fullPath = it.path().string();
					std::replace(fullPath.begin(), fullPath.end(), '\\', '/');

					UI::DragDropSource("AssetDragAndDrop", fullPath.c_str(), fullPath.size() + 1);
				}
				rightClickMenu();

				//char newName[64];
				//ImGui::InputText("##Renamed", newName, 64);
				ImGui::TextWrapped(name.c_str());
				ImGui::NextColumn();
			}
			ImGui::Columns(1);
		}
		void AssetBrowser::rightClickMenu() const
		{		
			if (ImGui::IsMouseClicked(ImGuiMouseButton_Right) && m_ViewportHovered)
			{
				ImGui::OpenPopup("RightClickMenu");
			}
			if (ImGui::BeginPopup("RightClickMenu"))
			{
				if (!m_RightClickedFile.empty())
				{
					const std::string fileName = m_RightClickedFile.string();
					if (ImGui::MenuItem("Rename"))
					{
						ImGui::CloseCurrentPopup();
					}
					if (ImGui::MenuItem("Delete"))
					{
						const std::string parentDir = m_CurrentDirectory.string();
						std::string fullPath  = parentDir + "\\" + fileName;
						//FileSystem::DeleteFileAtPath(fullPath);
						ImGui::CloseCurrentPopup();
					}
					if (Utils::GetExtension(fileName) == "png")
					{
						if (ImGui::MenuItem("Create Texture"))
						{
							std::string parentDir = m_CurrentDirectory.string();
							std::replace(parentDir.begin(), parentDir.end(), '\\', '/');

							const std::string fullpath = getUniqueAssetName("New Texture", ".tex");
							std::string fullImagePath = parentDir + "/" + fileName;
							//AssetManager::CreateAsset<Texture2D>(Utils::GetFilename(fullpath), parentDir, TextureSpecs{}, fullImagePath);
							ImGui::CloseCurrentPopup();
						}
					}
				}
				createAsset();			
				ImGui::EndPopup();
			}
		}
	
		size_t AssetBrowser::extensionToTexCoordsIndex(const std::string& extension) const
		{		
			auto it = m_FileTypeExtensions.find(extension);
			if (it != m_FileTypeExtensions.end())
				return it->second;
			return FileType::NumTypes;
		}
		void AssetBrowser::dragAndDrop(const std::filesystem::path& path) const
		{
			if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID))
			{
				std::string fullPath = s_AssetPath.string() + "/" + path.string();
				std::replace(fullPath.begin(), fullPath.end(), '\\', '/');
				const char* itemPath = fullPath.c_str();
				ImGui::SetDragDropPayload("ASSET_BROWSER_ITEM", itemPath, fullPath.size() + 1, ImGuiCond_Once);
				ImGui::EndDragDropSource();
			}
		}
		std::string AssetBrowser::getUniqueAssetName(const char* fileName, const char* extension) const
		{
			char fileNameTmp[60];
			std::string fullpath = m_CurrentDirectory.string() + "/" + fileName;
			std::replace(fullpath.begin(), fullpath.end(), '\\', '/');
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
				std::replace(fullpath.begin(), fullpath.end(), '\\', '/');
				while (std::filesystem::exists(fullpath))
				{
					if (extension)
						sprintf(fileNameTmp, "%s%d%s", fileName, index, extension);
					else
						sprintf(fileNameTmp, "%s%d", fileName, index);
					fullpath = m_CurrentDirectory.string() + "/" + fileNameTmp;
					std::replace(fullpath.begin(), fullpath.end(), '\\', '/');
					index++;
				}
			}
			return fullpath;
		}
		
		AssetBrowser::UV AssetBrowser::UV::Calculate(const glm::vec2& coords, const glm::vec2& cellSize, const glm::vec2& textureSize)
		{
			UV uv;
			uv.UV0 = { (coords.x * cellSize.x) / textureSize.x,
					 ((coords.y + 1) * cellSize.y) / textureSize.y };
			uv.UV1 = { ((coords.x + 1) * cellSize.x) / textureSize.x,
					   (coords.y * cellSize.y) / textureSize.y };

			return uv;
		}

	}
}