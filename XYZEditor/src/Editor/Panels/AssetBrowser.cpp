#include "stdafx.h"
#include "AssetBrowser.h"

#include "XYZ/Utils/StringUtils.h"
#include "XYZ/Utils/FileSystem.h"

#include "XYZ/Asset/AssetManager.h"
#include "XYZ/Scene/Scene.h"
#include "XYZ/Scene/SceneSerializer.h"
#include "XYZ/Renderer/Font.h"
#include "XYZ/Animation/Animation.h"
#include "XYZ/ImGui/ImGui.h"

#include "XYZ/Debug/Profiler.h"


#include "Editor/EditorData.h"

#include <imgui.h>


namespace XYZ {
	namespace Editor {

		static std::filesystem::path s_AssetPath = "Assets";

		AssetBrowser::AssetBrowser(std::string name)
			:
			EditorPanel(std::move(name)),
			m_BaseDirectory(s_AssetPath),
			m_CurrentDirectory(s_AssetPath),
			m_ViewportHovered(false),
			m_ViewportFocused(false)
		{
			registerFileTypeExtensions();

			m_IconSize = ImVec2(50.0f, 50.0f);
			m_ArrowSize = ImVec2(25.0f, 25.0f);

			m_Colors[ArrowColor]	= glm::vec4(0.6f, 0.6f, 0.6f, 1.0f);
			m_Colors[IconColor]		= glm::vec4(0.6f, 0.6f, 0.6f, 1.0f);
			m_Colors[DisabledColor] = glm::vec4(0.5f, 0.5f, 0.5f, 0.3f);
			m_Colors[HoverColor]    = glm::vec4(0.8f, 0.8f, 0.8f, 1.0f);
			m_Colors[ClickColor]	= glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
		}
	
		void AssetBrowser::OnImGuiRender(bool& open)
		{
			XYZ_PROFILE_FUNC("AssetBrowser::OnImGuiRender");
			if (ImGui::Begin("Asset Browser", &open))
			{
				m_ViewportFocused = ImGui::IsWindowFocused();
				m_ViewportHovered = ImGui::IsWindowHovered();
				renderTopPanel();

				static float w = 200.0f;
				UI::SplitterV(&w, "##DirectoryTree", "##CurrentDirectory",
					[&]() { XYZ_PROFILE_FUNC("AssetBrowser::processDirectoryTree");
							processDirectoryTree(m_BaseDirectory); },
					[&]() { XYZ_PROFILE_FUNC("AssetBrowser::processCurrentDirectory");
							processCurrentDirectory(); });
				
				//ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));
				//if (ImGui::BeginChild("##DirectoryTree", ImVec2(w, 0), true))
				//{
				//	XYZ_PROFILE_FUNC("AssetBrowser::processDirectoryTree");
				//	processDirectoryTree(m_BaseDirectory);
				//}
				//float h = ImGui::GetWindowHeight();
				//ImGui::EndChild();
				//ImGui::SameLine();
				//ImGui::InvisibleButton("vsplitter", ImVec2(8.0f, h));
				//if (ImGui::IsItemActive())
				//	w += ImGui::GetIO().MouseDelta.x;
				//ImGui::SameLine();
				//if (ImGui::BeginChild("##CurrentDirectory", ImVec2(0, 0), true))
				//{
				//	XYZ_PROFILE_FUNC("AssetBrowser::processCurrentDirectory");
				//	processCurrentDirectory();						
				//}
				//ImGui::EndChild();
				//ImGui::PopStyleVar();

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

		void AssetBrowser::SetBaseDirectory(const std::string& path)
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
				const std::string fullpath = createUniqueAssetName("New Folder", nullptr);
				FileSystem::CreateFolder(fullpath);
				ImGui::CloseCurrentPopup();
			}
			if (ImGui::MenuItem("Create Scene"))
			{
				const std::string fullpath = createUniqueAssetName("New Scene", ".xyz");
				Ref<XYZ::Scene> scene = Ref<XYZ::Scene>::Create("");

				ImGui::CloseCurrentPopup();
			}
			else if (ImGui::MenuItem("Create Animation"))
			{
				const std::string fullpath = createUniqueAssetName("New Animation", ".anim");
				Ref<XYZ::Animation> animation = Ref<XYZ::Animation>::Create();
				ImGui::CloseCurrentPopup();
			}
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

							const std::string fullpath = createUniqueAssetName("New Texture", ".tex");
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
	
		uint32_t AssetBrowser::extensionToTexCoordsIndex(const std::string& extension) const
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
		std::string AssetBrowser::createUniqueAssetName(const char* fileName, const char* extension) const
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

		void AssetBrowser::renderTopPanel()
		{
			const bool backArrowAvailable = m_CurrentDirectory != std::filesystem::path(s_AssetPath);
			const bool frontArrowAvailable = !m_DirectoriesVisited.empty();

			const ImVec4 backArrowColor = backArrowAvailable ? m_Colors[ArrowColor] : m_Colors[DisabledColor];
			const ImVec4 frontArrowColor = frontArrowAvailable ? m_Colors[ArrowColor] : m_Colors[DisabledColor];

			const UV& rightArrowTexCoords = EditorData::IconsSpriteSheet->GetTexCoords(Arrow);
			UV leftArrowTexCoords = rightArrowTexCoords;
			std::swap(leftArrowTexCoords.UV0.x, leftArrowTexCoords.UV1.x);

			ImGui::PushItemFlag(ImGuiItemFlags_Disabled, !backArrowAvailable);

			const bool backArrowPressed = UI::ImageButtonTransparent("##BackArrow", EditorData::IconsTexture->GetImage(), m_ArrowSize, m_Colors[HoverColor], m_Colors[ClickColor], backArrowColor,
				leftArrowTexCoords.UV0, leftArrowTexCoords.UV1);
			ImGui::PopItemFlag();
			ImGui::SameLine();


			ImGui::PushItemFlag(ImGuiItemFlags_Disabled, !frontArrowAvailable);
			const bool frontArrowPressed = UI::ImageButtonTransparent("##FrontArrow", EditorData::IconsTexture->GetImage(), m_ArrowSize, m_Colors[HoverColor], m_Colors[ClickColor], frontArrowColor,
				rightArrowTexCoords.UV0, rightArrowTexCoords.UV1);
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
		}

		void AssetBrowser::processCurrentDirectory()
		{
			if (!std::filesystem::is_directory(m_CurrentDirectory))
				return;

			static float padding = 32.0f;
			const float cellSize = m_IconSize.x + padding;

			const float panelWidth = ImGui::GetContentRegionAvail().x;
			int columnCount = (int)(panelWidth / cellSize);
			if (columnCount < 1)
				columnCount = 1;

			const UV& folderTexCoords = EditorData::IconsSpriteSheet->GetTexCoords(Folder);

			ImGui::Columns(columnCount, 0, false);
			for (auto& it : std::filesystem::directory_iterator(m_CurrentDirectory))
			{
				std::string name = it.path().filename().string();

				if (it.is_directory())
				{
					const bool dirPressed = UI::ImageButtonTransparent(name.c_str(), EditorData::IconsTexture->GetImage(), m_IconSize, m_Colors[HoverColor], m_Colors[ClickColor], m_Colors[IconColor],
						folderTexCoords.UV0, folderTexCoords.UV1);

					if (dirPressed)
					{
						m_CurrentDirectory /= it.path().filename();
						m_DirectoriesVisited.clear();
						break;
					}
				}
				else
				{
					const uint32_t index = extensionToTexCoordsIndex(Utils::GetExtension(name));
					if (index == FileType::NumTypes)
						continue;
					const UV& fileTexCoords = EditorData::IconsSpriteSheet->GetTexCoords(index);

					const bool iconPressed = UI::ImageButtonTransparent(name.c_str(), EditorData::IconsTexture->GetImage(), m_IconSize, m_Colors[HoverColor], m_Colors[ClickColor], m_Colors[IconColor],
						fileTexCoords.UV0, fileTexCoords.UV1);

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

				ImGui::TextWrapped(name.c_str());
				ImGui::NextColumn();
			}
			ImGui::Columns(1);
		}

		void AssetBrowser::processDirectoryTree(const std::filesystem::path& dirPath) const
		{
			const UV& folderTexCoords = EditorData::IconsSpriteSheet->GetTexCoords(Folder);

			const ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_OpenOnArrow;
			for (auto& it : std::filesystem::directory_iterator(dirPath))
			{
				if (it.is_directory())
				{
					std::string folderName = it.path().string();
					std::string folderID = "##" + folderName;
					const bool opened = ImGui::TreeNodeEx(folderID.c_str(), flags);
					ImGui::SameLine();
					UI::Image(EditorData::IconsTexture->GetImage(), { GImGui->Font->FontSize , GImGui->Font->FontSize }, folderTexCoords.UV0, folderTexCoords.UV1);
					ImGui::SameLine();
					ImGui::Text(Utils::GetFilename(folderName).c_str());
					if (opened)
					{
						processDirectoryTree(it.path());
						ImGui::TreePop();
					}
				}
			}
		}
	}
}