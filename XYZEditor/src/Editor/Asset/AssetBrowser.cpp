#include "stdafx.h"
#include "AssetBrowser.h"

#include "XYZ/Utils/StringUtils.h"
#include "XYZ/Utils/FileSystem.h"
#include "XYZ/Platform/Windows/WindowsFilesystem.cpp"

#include "XYZ/Asset/AssetManager.h"
#include "XYZ/Asset/AssimpModelImporter.h"
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
				EditorLayer::GetData().IconsSpriteSheet->GetTexCoords(ED::FolderIcon),
				nullptr, // Hover
				nullptr, // Left click
				[&](const std::filesystem::path& path) -> bool { m_FileManager.SetCurrentFile(path); return true; }, // Double left click
				[&](const std::filesystem::path& path) -> bool { return false; } // Right click
			});
			m_FileManager.RegisterExtension(Asset::GetExtension(AssetType::Texture), {
				EditorLayer::GetData().IconsTexture,
				EditorLayer::GetData().IconsSpriteSheet->GetTexCoords(ED::TextureIcon)
			});
			m_FileManager.RegisterExtension(Asset::GetExtension(AssetType::SubTexture), {
				EditorLayer::GetData().IconsTexture,
				EditorLayer::GetData().IconsSpriteSheet->GetTexCoords(ED::MeshIcon)
				});
			m_FileManager.RegisterExtension(Asset::GetExtension(AssetType::Material), {
				EditorLayer::GetData().IconsTexture,
				EditorLayer::GetData().IconsSpriteSheet->GetTexCoords(ED::MaterialIcon),
				nullptr, // Hover
				nullptr, // Left click
				[&](const std::filesystem::path& path) -> bool { assetSelected(path); return true; } // Double left click
				});
			m_FileManager.RegisterExtension(Asset::GetExtension(AssetType::Shader), {
				EditorLayer::GetData().IconsTexture,
				EditorLayer::GetData().IconsSpriteSheet->GetTexCoords(ED::ShaderIcon)
				});
			m_FileManager.RegisterExtension(Asset::GetExtension(AssetType::Script), {
				EditorLayer::GetData().IconsTexture,
				EditorLayer::GetData().IconsSpriteSheet->GetTexCoords(ED::ScriptIcon)
				});
			m_FileManager.RegisterExtension(Asset::GetExtension(AssetType::Animation), {
				EditorLayer::GetData().IconsTexture,
				EditorLayer::GetData().IconsSpriteSheet->GetTexCoords(ED::AnimationIcon)
				});
			m_FileManager.RegisterExtension("png", {
				EditorLayer::GetData().IconsTexture,
				EditorLayer::GetData().IconsSpriteSheet->GetTexCoords(ED::PngIcon),
				nullptr, // Hover
				nullptr, // Left click
				[&](const std::filesystem::path& path) -> bool { return true; }, // Double left click
				[&](const std::filesystem::path& path) -> bool { return assetRightClickMenuTextureSource(path); } // Right click
				});
			m_FileManager.RegisterExtension("jpg", {
				EditorLayer::GetData().IconsTexture,
				EditorLayer::GetData().IconsSpriteSheet->GetTexCoords(ED::JpgIcon),
				nullptr, // Hover
				nullptr, // Left click
				[&](const std::filesystem::path& path) -> bool { return true; }, // Double left click
				[&](const std::filesystem::path& path) -> bool { return assetRightClickMenuTextureSource(path); } // Right click
				});
			m_FileManager.RegisterExtension(Asset::GetExtension(AssetType::Mesh), {
				EditorLayer::GetData().IconsTexture,
				EditorLayer::GetData().IconsSpriteSheet->GetTexCoords(ED::MeshIcon),
				nullptr, // Hover
				nullptr, // Left click
				[&](const std::filesystem::path& path) -> bool { return true; }, // Double left click
				[&](const std::filesystem::path& path) -> bool { return assetRightClickMenuMESH(path); } // Right click
				});
			m_FileManager.RegisterExtension(Asset::GetExtension(AssetType::AnimatedMesh), {
				EditorLayer::GetData().IconsTexture,
				EditorLayer::GetData().IconsSpriteSheet->GetTexCoords(ED::MeshIcon),
				nullptr, // Hover
				nullptr, // Left click
				[&](const std::filesystem::path& path) -> bool { return true; }, // Double left click
				[&](const std::filesystem::path& path) -> bool { return assetRightClickMenuANIMMESH(path); } // Right click
				});
			m_FileManager.RegisterExtension(Asset::GetExtension(AssetType::MeshSource), {
				EditorLayer::GetData().IconsTexture,
				EditorLayer::GetData().IconsSpriteSheet->GetTexCoords(ED::MeshIcon),
				nullptr, // Hover
				nullptr, // Left click
				[&](const std::filesystem::path& path) -> bool { assetSelected(path); return true; }, // Double left click
				[&](const std::filesystem::path& path) -> bool { return assetRightClickMenuMESHSRC(path); } // Right click
				});
			m_FileManager.RegisterExtension(Asset::GetExtension(AssetType::Skeleton), {
				EditorLayer::GetData().IconsTexture,
				EditorLayer::GetData().IconsSpriteSheet->GetTexCoords(ED::MeshIcon)
				});
			m_FileManager.RegisterExtension(Asset::GetExtension(AssetType::AnimationController), {
				EditorLayer::GetData().IconsTexture,
				EditorLayer::GetData().IconsSpriteSheet->GetTexCoords(ED::MeshIcon),
				nullptr, // Hover
				nullptr, // Left click
				[&](const std::filesystem::path& path) -> bool { assetSelected(path); return true; }, // Double left click
				});
			m_FileManager.RegisterExtension("fbx", {
				EditorLayer::GetData().IconsTexture,
				EditorLayer::GetData().IconsSpriteSheet->GetTexCoords(ED::MeshIcon),
				nullptr, // Hover
				nullptr, // Left click
				[&](const std::filesystem::path& path) -> bool { return true; }, // Double left click
				[&](const std::filesystem::path& path) -> bool { return assetRightClickMenuFBX(path); } // Right click
				});
			m_FileManager.RegisterExtension("gltf", {
				EditorLayer::GetData().IconsTexture,
				EditorLayer::GetData().IconsSpriteSheet->GetTexCoords(ED::MeshIcon),
				nullptr, // Hover
				nullptr, // Left click
				[&](const std::filesystem::path& path) -> bool { return true; }, // Double left click
				[&](const std::filesystem::path& path) -> bool { return assetRightClickMenuFBX(path); } // Right click
				});
			m_FileManager.RegisterExtension(Asset::GetExtension(AssetType::Prefab), {
				EditorLayer::GetData().IconsTexture,
				EditorLayer::GetData().IconsSpriteSheet->GetTexCoords(ED::SceneIcon)
				});
			m_FileManager.RegisterExtension(Asset::GetExtension(AssetType::Scene), {
				EditorLayer::GetData().IconsTexture,
				EditorLayer::GetData().IconsSpriteSheet->GetTexCoords(ED::SceneIcon)
				});

			m_FileManager.RegisterExtension(Asset::GetExtension(AssetType::Plugin), {
				EditorLayer::GetData().IconsTexture,
				EditorLayer::GetData().IconsSpriteSheet->GetTexCoords(ED::SceneIcon)
				});

			m_FileManager.RegisterExtension("dll", {
				EditorLayer::GetData().IconsTexture,
				EditorLayer::GetData().IconsSpriteSheet->GetTexCoords(ED::CursorIcon)
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
					},
					[&]() { 
						XYZ_PROFILE_FUNC("AssetBrowser::processCurrentDirectory");
						m_FileManager.RenderCurrentDirectory("AssetDragAndDrop", m_IconSize);
						if (m_FileManager.GetRightClickedFile() == nullptr)
							rightClickMenu();
					});
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
			//if (!m_SelectedFile.empty())
			//{
			//	std::string fullFilePath = m_SelectedFile.string();
			//	std::replace(fullFilePath.begin(), fullFilePath.end(), '\\', '/');
			//	if (Utils::GetExtension(m_SelectedFile.string()) == "mat")
			//	{
			//		m_SelectedAsset = AssetManager::GetAsset<MaterialAsset>(std::filesystem::path(fullFilePath));				
			//		return m_SelectedAsset;
			//	}
			//}
			return Ref<Asset>();
		}

		
		bool AssetBrowser::rightClickMenu()
		{		
			if (ImGui::IsMouseClicked(ImGuiMouseButton_Right) && ImGui::IsWindowHovered())
			{
				ImGui::OpenPopup("CreateFileMenu");
			}
			if (ImGui::BeginPopup("CreateFileMenu"))
			{
				const std::string parentDir = m_FileManager.GetCurrentFile().GetPath().string();
				if (ImGui::MenuItem("Create Folder"))
				{
					const std::string fullpath = FileSystem::UniqueFilePath(parentDir, "New Folder", nullptr);
					FileSystem::CreateFolder(fullpath);
				}
				if (ImGui::MenuItem("Create Scene"))
				{
					const std::string fullpath = FileSystem::UniqueFilePath(parentDir, "New Scene", ".xyz");
					const std::string name = Utils::GetFilename(fullpath);
					AssetManager::CreateAsset<Scene>(name, parentDir, name);
				}
				if (ImGui::MenuItem("Create Material"))
				{
					const std::string fullpath = FileSystem::UniqueFilePath(parentDir, "New Material", ".mat");
					Ref<ShaderAsset> defaultShader = AssetManager::GetAsset<ShaderAsset>("Resources/Shaders/DefaultLitShader.shader");
					AssetManager::CreateAsset<MaterialAsset>(Utils::GetFilename(fullpath), parentDir, defaultShader);
				}
				if (ImGui::MenuItem("Create Animation Controller"))
				{
					const std::string fullpath = FileSystem::UniqueFilePath(parentDir, "New Controller", ".controller");
					AssetManager::CreateAsset<AnimationController>(Utils::GetFilename(fullpath), parentDir);
				}
				ImGui::EndPopup();
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
		void AssetBrowser::assetSelected(const std::filesystem::path& path)
		{
			AssetSelectedEvent ev(AssetManager::GetAsset<Asset>(path));
			Application::Get().OnEvent(ev);
		}
		bool AssetBrowser::assetRightClickMenuFBX(const std::filesystem::path& path)
		{
			if (ImGui::IsMouseClicked(ImGuiMouseButton_Right) && ImGui::IsWindowHovered())
			{
				ImGui::OpenPopup("RightClickMenu");
			}
			bool result = false;
			if (ImGui::BeginPopup("RightClickMenu"))
			{
				std::string name = Utils::GetFilenameWithoutExtension(path.string());
				if (assetRightClickMenu<MeshSource>(
					path,
					name + "." + Asset::GetExtension(AssetType::MeshSource),
					"Create Mesh Source",
					path.string()).Raw())
				{
					result = true;
				}
				if (assetRightClickMenu<SkeletonAsset>(
					path,
					name + "." + Asset::GetExtension(AssetType::Skeleton),
					"Create Skeleton",
					path.string()
					).Raw())
				{
					result = true;
				}

				if (ImGui::MenuItem("Create Assets"))
				{
					std::string parentDir = path.parent_path().string();
				
					AssimpModelImporter importer(parentDir, path.string());
					AssetManager::Serialize(importer.GetMeshSourceAsset()->GetHandle());
					AssetManager::Serialize(importer.GetSkeletonAsset()->GetHandle());
					
					for (const auto& animAsset : importer.GetAnimationAssets())
					{
						AssetManager::Serialize(animAsset->GetHandle());
					}
				}

				ImGui::EndPopup();
			}
			return result;
		}
		bool AssetBrowser::assetRightClickMenuMESHSRC(const std::filesystem::path& path)
		{
			if (ImGui::IsMouseClicked(ImGuiMouseButton_Right) && ImGui::IsWindowHovered())
			{
				ImGui::OpenPopup("RightClickMenu");
			}
			bool result = false;
			if (ImGui::BeginPopup("RightClickMenu"))
			{
				std::string name = Utils::GetFilenameWithoutExtension(path.string());
				Ref<MeshSource> meshSource = AssetManager::GetAsset<MeshSource>(path);

				if (assetRightClickMenu<Mesh>(
					path,
					name + "." + Asset::GetExtension(AssetType::Mesh),
					"Create Mesh",
					meshSource).Raw())
				{
					result = true;
				}

				if (meshSource->IsAnimated())
				{
					if (assetRightClickMenu<AnimatedMesh>(
						path,
						name + "." + Asset::GetExtension(AssetType::AnimatedMesh),
						"Create Animated Mesh",
						meshSource).Raw())
					{
						result = true;
					}
				}
				ImGui::EndPopup();
			}
			return result;
		}
		bool AssetBrowser::assetRightClickMenuMESH(const std::filesystem::path& path)
		{
			if (ImGui::IsMouseClicked(ImGuiMouseButton_Right) && ImGui::IsWindowHovered())
			{
				ImGui::OpenPopup("RightClickMenu");
			}
			bool result = false;
			if (ImGui::BeginPopup("RightClickMenu"))
			{
				std::string name = Utils::GetFilenameWithoutExtension(path.string());
				
				ImGui::EndPopup();
			}
			return result;
		}
		bool AssetBrowser::assetRightClickMenuANIMMESH(const std::filesystem::path& path)
		{
			if (ImGui::IsMouseClicked(ImGuiMouseButton_Right) && ImGui::IsWindowHovered())
			{
				ImGui::OpenPopup("RightClickMenu");
			}
			bool result = false;
			if (ImGui::BeginPopup("RightClickMenu"))
			{
				std::string name = Utils::GetFilenameWithoutExtension(path.string());
				Ref<AnimatedMesh> animMesh = AssetManager::GetAsset<AnimatedMesh>(path);
		
				if (assetRightClickMenu<Prefab>(
					path,
					name + "." + Asset::GetExtension(AssetType::Prefab),
					"Create Prefab",
					animMesh
					).Raw())
				{
					result = true;
				}
				ImGui::EndPopup();
			}
			return result;
		}
		bool AssetBrowser::assetRightClickMenuTextureSource(const std::filesystem::path& path)
		{
			if (ImGui::IsMouseClicked(ImGuiMouseButton_Right) && ImGui::IsWindowHovered())
			{
				ImGui::OpenPopup("RightClickMenu");
			}
			bool result = false;
			if (ImGui::BeginPopup("RightClickMenu"))
			{
				const std::string texturePath = path.string();
				const std::string name = Utils::GetFilenameWithoutExtension(texturePath);
				
				if (assetRightClickMenu<Texture2D>(
					path,
					name + "." + Asset::GetExtension(AssetType::Texture),
					"Create Texture",
					texturePath
					).Raw())
				{
					result = true;
				}
				ImGui::EndPopup();
			}
			return result;
		}
	}
}