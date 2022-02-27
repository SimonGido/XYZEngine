#include "stdafx.h"
#include "AssetBrowser.h"

#include "XYZ/Utils/StringUtils.h"
#include "XYZ/Utils/FileSystem.h"

#include "XYZ/Asset/AssetManager.h"
#include "XYZ/Scene/Scene.h"
#include "XYZ/Scene/SceneSerializer.h"
#include "XYZ/Renderer/Font.h"
#include "XYZ/ImGui/ImGui.h"

#include "XYZ/Debug/Profiler.h"

#include "EditorLayer.h"
#include "Editor/Event/EditorEvents.h"

#include <imgui.h>


namespace XYZ {
	namespace Editor {

		static std::filesystem::path s_AssetPath = "Assets";



		AssetBrowser::AssetBrowser(std::string name)
			:
			EditorPanel(std::move(name)),
			m_BaseDirectory(s_AssetPath),
			m_DirectoryTree(s_AssetPath),
			m_SplitterWidth(200.0f)
		{
			m_IconSize = ImVec2(50.0f, 50.0f);
			m_ArrowSize = ImVec2(25.0f, 25.0f);		

			AssetManager::GetFileWatcher()->AddOnFileChange<&AssetBrowser::onFileChange>(this);
			AssetManager::GetFileWatcher()->AddOnFileAdded<&AssetBrowser::onFileAdded>(this);
			AssetManager::GetFileWatcher()->AddOnFileRemoved<&AssetBrowser::onFileRemoved>(this);
			AssetManager::GetFileWatcher()->AddOnFileRenamed<&AssetBrowser::onFileRenamed>(this);
		}

		AssetBrowser::~AssetBrowser()
		{
			AssetManager::GetFileWatcher()->RemoveOnFileChange<&AssetBrowser::onFileChange>(this);
			AssetManager::GetFileWatcher()->RemoveOnFileAdded<&AssetBrowser::onFileAdded>(this);
			AssetManager::GetFileWatcher()->RemoveOnFileRemoved<&AssetBrowser::onFileRemoved>(this);
			AssetManager::GetFileWatcher()->RemoveOnFileRenamed<&AssetBrowser::onFileRenamed>(this);
		}
	
		void AssetBrowser::OnImGuiRender(bool& open)
		{
			XYZ_PROFILE_FUNC("AssetBrowser::OnImGuiRender");
			if (ImGui::Begin("Asset Browser", &open))
			{
				renderTopPanel();

				UI::SplitterV(&m_SplitterWidth, "##DirectoryTree", "##CurrentDirectory",
					[&]() { XYZ_PROFILE_FUNC("AssetBrowser::processDirectoryTree");
							processDirectoryTree(m_DirectoryTree.GetRoot()); },
					[&]() { XYZ_PROFILE_FUNC("AssetBrowser::processCurrentDirectory");
							processCurrentDirectory(); });
				
				rightClickMenu();

				if (ImGui::GetIO().MouseReleased[ImGuiMouseButton_Left]
					&& ImGui::IsWindowFocused()
					&& ImGui::IsWindowHovered())
				{
					m_RightClickedFile.clear();
					m_SelectedFile.clear();
				}
				if (m_RebuildDirectoryTree)
				{
					m_RebuildDirectoryTree = false;
					m_DirectoryTree.Rebuild(s_AssetPath);
				}
			}
			ImGui::End();
		}

		void AssetBrowser::SetBaseDirectory(const std::string& path)
		{
			m_BaseDirectory = path;
			m_DirectoryTree = DirectoryTree(path);
		}

		Ref<Asset> AssetBrowser::GetSelectedAsset() const
		{
			if (!m_SelectedFile.empty())
			{
				std::string fullFilePath = m_SelectedFile.string();
				std::replace(fullFilePath.begin(), fullFilePath.end(), '\\', '/');
				if (Utils::GetExtension(m_SelectedFile.string()) == "mat")
				{
					return AssetManager::GetAsset<MaterialAsset>(std::filesystem::path(fullFilePath));
				}
			}
			return Ref<Asset>();
		}

		void AssetBrowser::createAsset()
		{
			std::string parentDir = m_DirectoryTree.GetCurrentNode().GetStrPath();
			if (ImGui::MenuItem("Create Folder"))
			{
				const std::string fullpath = FileSystem::UniqueFilePath(parentDir, "New Folder", nullptr);
				FileSystem::CreateFolder(fullpath);
				ImGui::CloseCurrentPopup();
			}
			if (ImGui::MenuItem("Create Scene"))
			{
				const std::string fullpath = FileSystem::UniqueFilePath(parentDir, "New Scene", ".xyz");
				Ref<XYZ::Scene> scene = Ref<XYZ::Scene>::Create("");

				ImGui::CloseCurrentPopup();
			}
			if (ImGui::MenuItem("Create Material"))
			{
				const std::string fullpath = FileSystem::UniqueFilePath(parentDir, "New Material", ".mat");
				Ref<ShaderAsset> defaultShader = AssetManager::GetAsset<ShaderAsset>("Resources/Shaders/DefaultLitShader.shader");
				AssetManager::CreateAsset<MaterialAsset>(Utils::GetFilename(fullpath), parentDir, defaultShader);
				m_RebuildDirectoryTree = true;
			}
		}
		
		void AssetBrowser::rightClickMenu()
		{		
			if (ImGui::IsMouseClicked(ImGuiMouseButton_Right) && ImGui::IsWindowHovered())
			{
				ImGui::OpenPopup("RightClickMenu");
			}
			if (ImGui::BeginPopup("RightClickMenu"))
			{
				if (!m_RightClickedFile.empty())
				{
					const std::string fileName = m_RightClickedFile.string();
					std::string ext = Utils::GetExtension(fileName);
					if (ext == "png" || ext == "jpg")
					{
						if (ImGui::MenuItem("Create Texture"))
						{
							std::string parentDir = m_DirectoryTree.GetCurrentNode().GetStrPath();
							const std::string fullpath = FileSystem::UniqueFilePath(parentDir, "New Texture", ".tex");
							std::string fullImagePath = parentDir + "/" + fileName;
							AssetManager::CreateAsset<Texture2D>(Utils::GetFilename(fullpath), parentDir, fullImagePath);
							ImGui::CloseCurrentPopup();
						}
					}
					else if (ext == "fbx")
					{
						if (ImGui::MenuItem("Create Mesh Source"))
						{
							std::string parentDir = m_DirectoryTree.GetCurrentNode().GetStrPath();
							const std::string fullpath = FileSystem::UniqueFilePath(parentDir, "New Mesh Source", ".meshsrc");
							std::string fullModelPath = parentDir + "/" + fileName;
							AssetManager::CreateAsset<MeshSource>(Utils::GetFilename(fullpath), parentDir, fullModelPath);
							ImGui::CloseCurrentPopup();
						}
					}
					else if (ext == "tex")
					{
						if (ImGui::MenuItem("Create SubTexture"))
						{
							std::string parentDir = m_DirectoryTree.GetCurrentNode().GetStrPath();
							const std::string fullpath = FileSystem::UniqueFilePath(parentDir, "New SubTexture", ".subtex");
							std::string fullTexturePath = parentDir + "/" + fileName;
							Ref<Texture2D> texture = AssetManager::GetAsset<Texture2D>(std::filesystem::path(fullTexturePath));
							AssetManager::CreateAsset<SubTexture>(Utils::GetFilename(fullpath), parentDir, texture);
							ImGui::CloseCurrentPopup();
						}
					}
				}
				createAsset();			
				ImGui::EndPopup();
			}
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


		void AssetBrowser::renderTopPanel()
		{
			const auto& preferences = EditorLayer::GetData();
			const bool backArrowAvailable = !m_DirectoryTree.UndoEmpty();
			const bool frontArrowAvailable = !m_DirectoryTree.RedoEmpty();

			const ImVec4 backArrowColor = backArrowAvailable ? preferences.Color[ED::IconColor] : preferences.Color[ED::DisabledColor];
			const ImVec4 frontArrowColor = frontArrowAvailable ? preferences.Color[ED::IconColor] : preferences.Color[ED::DisabledColor];

			const UV& rightArrowTexCoords = EditorLayer::GetData().IconsSpriteSheet->GetTexCoords(ED::ArrowIcon);
			UV leftArrowTexCoords = rightArrowTexCoords;
			std::swap(leftArrowTexCoords[0].x, leftArrowTexCoords[1].x);

			ImGui::PushItemFlag(ImGuiItemFlags_Disabled, !backArrowAvailable);

			const bool backArrowPressed = UI::ImageButtonTransparent("##BackArrow", preferences.IconsTexture->GetImage(), m_ArrowSize, preferences.Color[ED::IconHoverColor], preferences.Color[ED::IconClickColor], backArrowColor,
				leftArrowTexCoords[0], leftArrowTexCoords[1]);
			ImGui::PopItemFlag();
			ImGui::SameLine();


			ImGui::PushItemFlag(ImGuiItemFlags_Disabled, !frontArrowAvailable);
			const bool frontArrowPressed = UI::ImageButtonTransparent("##FrontArrow", preferences.IconsTexture->GetImage(), m_ArrowSize, preferences.Color[ED::IconHoverColor], preferences.Color[ED::IconClickColor], frontArrowColor,
				rightArrowTexCoords[0], rightArrowTexCoords[1]);
			ImGui::PopItemFlag();
			ImGui::SameLine();


			if (backArrowPressed && backArrowAvailable)
				m_DirectoryTree.Undo();
			
			if (frontArrowPressed && frontArrowAvailable)
				m_DirectoryTree.Redo();

			std::string path = m_DirectoryTree.GetCurrentNode().GetStrPath();
			UI::Utils::SetPathBuffer(path);
			if (ImGui::InputText("###", UI::Utils::GetPathBuffer(), _MAX_PATH))
				m_DirectoryTree.SetCurrentNode(UI::Utils::GetPathBuffer());
		}

		void AssetBrowser::processCurrentDirectory()
		{
			const auto&  preferences = EditorLayer::GetData();
			static float padding = 32.0f;
			const float  cellSize = m_IconSize.x + padding;

			const float panelWidth = ImGui::GetContentRegionAvail().x;
			int columnCount = (int)(panelWidth / cellSize);
			columnCount = std::max(1, columnCount);
			
			ImGui::Columns(columnCount, 0, false);
			for (const auto& node : m_DirectoryTree.GetCurrentNode())
			{
				UI::ScopedID id(node.GetName().c_str());
				const bool pressed = node.OnImGuiRender(m_IconSize);

				if (node.IsDirectory())
				{
					if (pressed)
					{
						m_DirectoryTree.SetCurrentNode(node);
						break;
					}
				}
				else
				{
					if (pressed && !ImGui::IsMouseDragging(ImGuiMouseButton_Left))
					{
						m_SelectedFile = node.GetPath();
						auto asset = GetSelectedAsset();
						Application::Get().OnEvent(AssetSelectedEvent(asset));
					}
					else if (ImGui::IsItemClicked(ImGuiMouseButton_Right))
					{
						m_RightClickedFile = node.GetPath();
					}
					std::string path = node.GetStrPath();
					UI::DragDropSource("AssetDragAndDrop", path.c_str(), path.size() + 1);
				}		
				ImGui::TextWrapped(node.GetName().c_str());
				ImGui::NextColumn();
			}
			ImGui::Columns(1);

		}

		void AssetBrowser::processDirectoryTree(const DirectoryNode& parentNode)
		{
			const auto& preferences = EditorLayer::GetData();
			const UV& folderTexCoords = EditorLayer::GetData().IconsSpriteSheet->GetTexCoords(ED::FolderIcon);

			const ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_OpenOnArrow;
				
			for (const auto& node : parentNode)
			{
				if (node.IsDirectory())
				{
					std::string folderID = "##" + node.GetName();
					const bool opened = ImGui::TreeNodeEx(folderID.c_str(), flags);
					if (ImGui::IsItemClicked())
						m_DirectoryTree.SetCurrentNode(node);

					ImGui::SameLine();
					UI::Image(preferences.IconsTexture->GetImage(), { GImGui->Font->FontSize , GImGui->Font->FontSize }, folderTexCoords[0], folderTexCoords[1]);
					ImGui::SameLine();
					ImGui::Text(node.GetName().c_str());

					if (opened)
					{
						processDirectoryTree(node);
						ImGui::TreePop();
					}
				}
			}
		}
		void AssetBrowser::onFileChange(const std::wstring& filePath)
		{
			XYZ_PROFILE_FUNC("AssetBrowser::onFileChange");
			m_RebuildDirectoryTree = true;
		}
		void AssetBrowser::onFileAdded(const std::wstring& filePath)
		{
			XYZ_PROFILE_FUNC("AssetBrowser::onFileAdded");
			m_RebuildDirectoryTree = true;
		}
		void AssetBrowser::onFileRemoved(const std::wstring& filePath)
		{
			XYZ_PROFILE_FUNC("AssetBrowser::onFileRemoved");
			m_DirectoryTree.Rebuild(s_AssetPath);
		}
		void AssetBrowser::onFileRenamed(const std::wstring& filePath)
		{
			XYZ_PROFILE_FUNC("AssetBrowser::onFileRenamed");
			m_DirectoryTree.Rebuild(s_AssetPath);
		}
	}
}