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


		AssetBrowser::AssetBrowser(std::string name)
			:
			EditorPanel(std::move(name)),
			m_BaseDirectory(AssetManager::GetAssetDirectory()),
			m_FileManager(AssetManager::GetAssetDirectory()),
			m_SplitterWidth(200.0f)
		{
			m_IconSize = ImVec2(50.0f, 50.0f);
			m_ArrowSize = ImVec2(25.0f, 25.0f);		

			AssetManager::GetFileWatcher()->AddOnFileChanged<&AssetBrowser::onFileChange>(this);
		

			m_FileManager.RegisterExtension("dir", {
				EditorLayer::GetData().IconsTexture,
				EditorLayer::GetData().IconsSpriteSheet->GetTexCoords(ED::FolderIcon)
			});
			m_FileManager.RegisterExtension("tex", {
				EditorLayer::GetData().IconsTexture,
				EditorLayer::GetData().IconsSpriteSheet->GetTexCoords(ED::TextureIcon)
			});
			m_FileManager.RegisterExtension("subtex", {
				EditorLayer::GetData().IconsTexture,
				EditorLayer::GetData().IconsSpriteSheet->GetTexCoords(ED::MeshIcon)
				});
			m_FileManager.RegisterExtension("mat", {
				EditorLayer::GetData().IconsTexture,
				EditorLayer::GetData().IconsSpriteSheet->GetTexCoords(ED::MaterialIcon)
				});
			m_FileManager.RegisterExtension("shader", {
				EditorLayer::GetData().IconsTexture,
				EditorLayer::GetData().IconsSpriteSheet->GetTexCoords(ED::ShaderIcon)
				});
			m_FileManager.RegisterExtension("cs", {
				EditorLayer::GetData().IconsTexture,
				EditorLayer::GetData().IconsSpriteSheet->GetTexCoords(ED::ScriptIcon)
				});
			m_FileManager.RegisterExtension("anim", {
				EditorLayer::GetData().IconsTexture,
				EditorLayer::GetData().IconsSpriteSheet->GetTexCoords(ED::AnimationIcon)
				});
			m_FileManager.RegisterExtension("png", {
				EditorLayer::GetData().IconsTexture,
				EditorLayer::GetData().IconsSpriteSheet->GetTexCoords(ED::PngIcon)
				});
			m_FileManager.RegisterExtension("jpg", {
				EditorLayer::GetData().IconsTexture,
				EditorLayer::GetData().IconsSpriteSheet->GetTexCoords(ED::JpgIcon)
				});
			m_FileManager.RegisterExtension("mesh", {
				EditorLayer::GetData().IconsTexture,
				EditorLayer::GetData().IconsSpriteSheet->GetTexCoords(ED::MeshIcon)
				});
			m_FileManager.RegisterExtension("fbx", {
				EditorLayer::GetData().IconsTexture,
				EditorLayer::GetData().IconsSpriteSheet->GetTexCoords(ED::MeshIcon)
				});
		}

		AssetBrowser::~AssetBrowser()
		{
			AssetManager::GetFileWatcher()->RemoveOnFileChanged<&AssetBrowser::onFileChange>(this);
		}
	
		void AssetBrowser::OnImGuiRender(bool& open)
		{
			XYZ_PROFILE_FUNC("AssetBrowser::OnImGuiRender");
			if (ImGui::Begin("Asset Browser", &open))
			{
				renderTopPanel();

				UI::SplitterV(&m_SplitterWidth, "##DirectoryTree", "##CurrentDirectory",
					[&]() { 
						XYZ_PROFILE_FUNC("AssetBrowser::processDirectoryTree");
						m_FileManager.RenderDirectoryTree();
						tryClearClickedFiles();
					},
					[&]() { 
						XYZ_PROFILE_FUNC("AssetBrowser::processCurrentDirectory");
						m_FileManager.RenderCurrentDirectory("AssetDragAndDrop", m_IconSize);
						tryClearClickedFiles();
					});
				tryClearClickedFiles();
			}
			ImGui::End();
		}

		void AssetBrowser::SetBaseDirectory(const std::string& path)
		{
			m_BaseDirectory = path;
			m_FileManager = ImGuiFileManager(path);
		}

		Ref<Asset> AssetBrowser::GetSelectedAsset()
		{
			if (!m_SelectedFile.empty())
			{
				std::string fullFilePath = m_SelectedFile.string();
				std::replace(fullFilePath.begin(), fullFilePath.end(), '\\', '/');
				if (Utils::GetExtension(m_SelectedFile.string()) == "mat")
				{
					m_SelectedAsset = AssetManager::GetAsset<MaterialAsset>(std::filesystem::path(fullFilePath));				
					return m_SelectedAsset;
				}
			}
			return Ref<Asset>();
		}

		void AssetBrowser::createAsset()
		{
			//std::string parentDir = m_DirectoryTree.GetCurrentNode().GetPathString();
			//if (ImGui::MenuItem("Create Folder"))
			//{
			//	const std::string fullpath = FileSystem::UniqueFilePath(parentDir, "New Folder", nullptr);
			//	FileSystem::CreateFolder(fullpath);
			//}
			//if (ImGui::MenuItem("Create Scene"))
			//{
			//	const std::string fullpath = FileSystem::UniqueFilePath(parentDir, "New Scene", ".xyz");
			//	Ref<XYZ::Scene> scene = Ref<XYZ::Scene>::Create(Utils::GetFilenameWithoutExtension(fullpath));
			//}
			//if (ImGui::MenuItem("Create Material"))
			//{
			//	const std::string fullpath = FileSystem::UniqueFilePath(parentDir, "New Material", ".mat");
			//	Ref<ShaderAsset> defaultShader = AssetManager::GetAsset<ShaderAsset>("Resources/Shaders/DefaultLitShader.shader");
			//	AssetManager::CreateAsset<MaterialAsset>(Utils::GetFilename(fullpath), parentDir, defaultShader);
			//}
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
					std::string ext = Utils::GetExtension(m_RightClickedFile.string());
					if (ext == "png" || ext == "jpg")
					{
						if (ImGui::MenuItem("Create Texture"))
						{
							//std::string parentDir = m_DirectoryTree.GetCurrentNode().GetPathString();
							//const std::string fullpath = FileSystem::UniqueFilePath(parentDir, "New Texture", ".tex");
							//AssetManager::CreateAsset<Texture2D>(Utils::GetFilename(fullpath), parentDir, m_RightClickedFile.string());
						}
					}
					else if (ext == "fbx")
					{
						if (ImGui::MenuItem("Create Mesh Source"))
						{
							//std::string parentDir = m_DirectoryTree.GetCurrentNode().GetPathString();
							//const std::string fullpath = FileSystem::UniqueFilePath(parentDir, "New Mesh Source", ".meshsrc");
							//AssetManager::CreateAsset<MeshSource>(Utils::GetFilename(fullpath), parentDir, m_RightClickedFile.string());
						}
					}
					else if (ext == "tex")
					{
						if (ImGui::MenuItem("Create SubTexture"))
						{
							//std::string parentDir = m_DirectoryTree.GetCurrentNode().GetPathString();
							//const std::string fullpath = FileSystem::UniqueFilePath(parentDir, "New SubTexture", ".subtex");
							//Ref<Texture2D> texture = AssetManager::GetAsset<Texture2D>(m_RightClickedFile);
							//AssetManager::CreateAsset<SubTexture>(Utils::GetFilename(fullpath), parentDir, texture);
						}
					}
				}
				createAsset();			
				ImGui::EndPopup();
			}
		}

		bool AssetBrowser::tryClearClickedFiles()
		{
			if (ImGui::GetIO().MouseReleased[ImGuiMouseButton_Left]
				&& ImGui::IsWindowFocused()
				&& ImGui::IsWindowHovered())
			{
				m_RightClickedFile.clear();
				m_SelectedFile.clear();
				return true;
			}
			return false;
		}

		void AssetBrowser::renderTopPanel()
		{
			const auto& preferences = EditorLayer::GetData();
			const bool backArrowAvailable = !m_FileManager.IsUndoEmpty();
			const bool frontArrowAvailable = !m_FileManager.IsRedoEmpty();

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
				m_FileManager.Undo();
			
			if (frontArrowPressed && frontArrowAvailable)
				m_FileManager.Redo();

			std::string path = m_FileManager.GetCurrentFile().GetPath().string();
			UI::Utils::SetPathBuffer(path);
			if (ImGui::InputText("###", UI::Utils::GetPathBuffer(), _MAX_PATH))
				m_FileManager.SetCurrentFile(UI::Utils::GetPathBuffer());
		}

		//void AssetBrowser::processCurrentDirectory()
		//{
		//	const auto&  preferences = EditorLayer::GetData();
		//	static float padding = 32.0f;
		//	const float  cellSize = m_IconSize.x + padding;
		//
		//	const float panelWidth = ImGui::GetContentRegionAvail().x;
		//	int columnCount = (int)(panelWidth / cellSize);
		//	columnCount = std::max(1, columnCount);
		//	
		//	ImGui::Columns(columnCount, 0, false);
		//	for (auto& node : m_DirectoryTree.GetCurrentNode())
		//	{
		//		UI::ScopedID id(node.GetName().c_str());
		//
		//		bool leftClicked = false;
		//		bool rightClicked = false;
		//		bool leftDoubleClicked = false;
		//		
		//		node.OnImGuiRender("AssetDragAndDrop", m_IconSize, leftClicked, rightClicked, leftDoubleClicked);
		//		
		//		if (leftDoubleClicked)
		//		{
		//			if (node.IsDirectory())
		//			{
		//				m_DirectoryTree.SetCurrentNode(node);
		//				break;
		//			}
		//			else
		//			{
		//				m_SelectedFile = node.GetPath();
		//				auto asset = GetSelectedAsset();
		//				Application::Get().OnEvent(AssetSelectedEvent(asset));
		//			}
		//		}
		//		else if (leftClicked)
		//		{
		//		}
		//		else if (rightClicked)
		//		{
		//			m_RightClickedFile = node.GetPath();
		//		}
		//				
		//		ImGui::NextColumn();
		//	}
		//	ImGui::Columns(1);
		//	rightClickMenu();
		//}

		//void AssetBrowser::processDirectoryTree(const File& file, bool defaulOpen)
		//{
		//	const auto& preferences = EditorLayer::GetData();
		//	const UV& folderTexCoords = EditorLayer::GetData().IconsSpriteSheet->GetTexCoords(ED::FolderIcon);
		//
		//	ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_OpenOnArrow;
		//	if (defaulOpen)
		//		flags |= ImGuiTreeNodeFlags_DefaultOpen;
		//
		//	if (file.Empty())
		//		flags |= ImGuiTreeNodeFlags_Leaf;
		//
		//
		//	if (file.IsDirectory())
		//	{
		//		std::string folderID = "##" + file.GetName();
		//		const bool opened = ImGui::TreeNodeEx(folderID.c_str(), flags);
		//
		//		if (UI::Utils::IsItemDoubleClicked(ImGuiMouseButton_Left))
		//			m_FileManager.SetCurrentFile(file.GetPath());
		//
		//		ImGui::SameLine();
		//		UI::Image(preferences.IconsTexture->GetImage(), { GImGui->Font->FontSize , GImGui->Font->FontSize }, folderTexCoords[0], folderTexCoords[1]);
		//		ImGui::SameLine();
		//		ImGui::Text(file.GetName().c_str());
		//
		//		if (opened)
		//		{
		//			for (auto& node : file)
		//				processDirectoryTree(node);
		//			ImGui::TreePop();
		//		}
		//	}
		//}
		
		void AssetBrowser::onFileChange(FileWatcher::ChangeType type, const std::filesystem::path& filePath)
		{
			if (type == FileWatcher::ChangeType::Added)
			{		
				m_FileManager.AddFile(filePath);
			}
			else if (type == FileWatcher::ChangeType::Removed)
			{
				m_FileManager.RemoveFile(filePath);
			}
		}
	}
}