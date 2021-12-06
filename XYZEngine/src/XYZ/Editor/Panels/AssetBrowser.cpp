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

		static glm::vec4 CalculateTexCoords(const glm::vec2& coords, const glm::vec2& size, const glm::vec2& textureSize)
		{
			return {
				(coords.x * size.x) / textureSize.x,
				((coords.y + 1) * size.y) / textureSize.y,
				((coords.x + 1) * size.x) / textureSize.x,
				(coords.y * size.y) / textureSize.y,
			};
		}

		static ImVec2 ToImVec2(glm::vec2 val)
		{
			return { val.x , val.y };
		}


		struct AssetBrowserStyle
		{
			struct UV
			{
				ImVec2 UV0, UV1;
			};

			void Init(const glm::vec2& cellSize, const glm::vec2& textureSize);		
			static UV CalculateTexCoords(const glm::vec2& coords, const glm::vec2& size, const glm::vec2& textureSize);

			enum FileType
			{
				Arrow,
				Folder,
				Scene,
				Texture,
				SubTexture,
				Material,
				Shader,
				Font,
				Audio,
				Script,
				Mesh,
				Animation,
				Png,
				Jpg,
				NumTypes
			};
			enum Color
			{
				ArrowColor,
				ArrowInvalidColor,
				FolderColor,
				NumColors
			};

			ImVec2 IconSize;
			ImVec2 ArrowSize;		
			UV	   TexCoords[NumTypes];
			ImVec4 Colors[NumColors];
		};


		void AssetBrowserStyle::Init(const glm::vec2& cellSize, const glm::vec2& textureSize)
		{
			IconSize = ImVec2(50.0f, 50.0f);
			ArrowSize = ImVec2(25.0f, 25.0f);

			TexCoords[Arrow]		= CalculateTexCoords(glm::vec2(0, 1), cellSize, textureSize);
			TexCoords[Folder]		= CalculateTexCoords(glm::vec2(0, 3), cellSize, textureSize);
			TexCoords[Shader]		= CalculateTexCoords(glm::vec2(1, 3), cellSize, textureSize);
			TexCoords[Script]		= CalculateTexCoords(glm::vec2(3, 3), cellSize, textureSize);
			TexCoords[Material]		= CalculateTexCoords(glm::vec2(2, 3), cellSize, textureSize);
			TexCoords[Texture]		= CalculateTexCoords(glm::vec2(2, 2), cellSize, textureSize);
			TexCoords[Mesh]			= CalculateTexCoords(glm::vec2(1, 2), cellSize, textureSize);
			TexCoords[Scene]		= CalculateTexCoords(glm::vec2(1, 1), cellSize, textureSize);
			TexCoords[Animation]	= CalculateTexCoords(glm::vec2(0, 1), cellSize, textureSize);
			
			Colors[ArrowColor]		  = ImVec4(0.5f, 0.5f, 0.5f, 1.0f);
			Colors[ArrowInvalidColor] = ImVec4(0.3f, 0.3f, 0.3f, 0.3f);
		}

		AssetBrowserStyle::UV AssetBrowserStyle::CalculateTexCoords(const glm::vec2& coords, const glm::vec2& size, const glm::vec2& textureSize)
		{
			UV uv;
			uv.UV0 = { (coords.x * size.x) / textureSize.x,
					 ((coords.y + 1) * size.y) / textureSize.y };
			uv.UV1 = { ((coords.x + 1) * size.x) / textureSize.x,
					   (coords.y * size.y) / textureSize.y };
			
			return uv;
		}

		static AssetBrowserStyle s_Style;
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
			const glm::vec2 size = glm::vec2(width / divisor, height / divisor);
			s_Style.Init(size, { width, height });
		}
	
		void AssetBrowser::OnImGuiRender(bool& open)
		{
			if (ImGui::Begin("Asset Browser", &open))
			{
				m_ViewportFocused = ImGui::IsWindowFocused();
				m_ViewportHovered = ImGui::IsWindowHovered();

				const bool backArrowAvailable = m_CurrentDirectory != std::filesystem::path(s_AssetPath);
				const bool frontArrowAvailable = !m_DirectoriesVisited.empty();

				ImVec4 arrowColor = backArrowAvailable ? s_Style.Colors[AssetBrowserStyle::ArrowColor] : s_Style.Colors[AssetBrowserStyle::ArrowInvalidColor];
	
				if (UI::ImageButtonTransparent("##BackArrow", m_Texture->GetImage(),
					s_Style.ArrowSize,
					ImVec4(1, 1, 1, 1), ImVec4(1, 1, 1, 1), ImVec4(1, 1, 1, 1),
					
					ImVec2(m_TexCoords[Arrow].z, m_TexCoords[Arrow].y),
					ImVec2(m_TexCoords[Arrow].x, m_TexCoords[Arrow].w)) && backArrowAvailable)
				{
					m_DirectoriesVisited.push_front(m_CurrentDirectory);
					m_CurrentDirectory = m_CurrentDirectory.parent_path();
				}
				ImGui::SameLine();
				
				arrowColor = frontArrowAvailable ? m_Colors[ArrowColor] : m_Colors[ArrowInvalidColor];
			
				if (UI::ImageButtonTransparent("##BackArrow", m_Texture->GetImage(),
					ToImVec2(m_ArrowSize),
					ImVec4(1, 1, 1, 1), ImVec4(1, 1, 1, 1), ImVec4(1, 1, 1, 1),
					ImVec2(m_TexCoords[Arrow].x, m_TexCoords[Arrow].y),
					ImVec2(m_TexCoords[Arrow].z, m_TexCoords[Arrow].w)) && frontArrowAvailable)
				{
					m_CurrentDirectory = m_DirectoriesVisited.front();
					m_DirectoriesVisited.pop_front();
				}
				ImGui::SameLine();
			
				UI::Utils::SetPathBuffer(m_CurrentDirectory.string());
				if (ImGui::InputText("###", UI::Utils::GetPathBuffer(), _MAX_PATH))
				{
					m_CurrentDirectory = UI::Utils::GetPathBuffer();
					m_DirectoriesVisited.clear();
				}

				//processDirectory();

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
			m_FileTypeExtensions["xyz"]    = Type::Scene;
			m_FileTypeExtensions["tex"]    = Type::Texture;
			m_FileTypeExtensions["subtex"] = Type::SubTexture;
			m_FileTypeExtensions["mat"]    = Type::Material;
			m_FileTypeExtensions["shader"] = Type::Shader;
			m_FileTypeExtensions["cs"]	   = Type::Script;
			m_FileTypeExtensions["anim"]   = Type::Animation;
			m_FileTypeExtensions["png"]    = Type::Png;
			m_FileTypeExtensions["jpg"]    = Type::Jpg;
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
				ImGui::PushID(name.c_str());
				
				if (it.is_directory())
				{
					if (ImGui::ImageButton((void*)(uint64_t)m_Texture.Raw(), { m_IconSize.x, m_IconSize.y }, { m_TexCoords[Folder].x, m_TexCoords[Folder].y }, { m_TexCoords[Folder].z, m_TexCoords[Folder].w }))
					{
						m_CurrentDirectory /= it.path().filename();
						m_DirectoriesVisited.clear();
					}
				}
				else
				{
					const size_t index = extensionToTexCoordsIndex(Utils::GetExtension(name));
					if (index == Type::NumTypes)
					{
						ImGui::PopID();
						continue;
					}
					if (ImGui::ImageButton((void*)(uint64_t)m_Texture.Raw(), { m_IconSize.x, m_IconSize.y }, { m_TexCoords[index].x, m_TexCoords[index].y }, { m_TexCoords[index].z, m_TexCoords[index].w }))
					{
						m_SelectedFile = name;
					}
					if (ImGui::IsItemClicked(ImGuiMouseButton_Right))
					{
						m_RightClickedFile = name;
					}
					dragAndDrop(std::filesystem::relative(it, s_AssetPath));
				}
				rightClickMenu();

				//char newName[64];
				//ImGui::InputText("##Renamed", newName, 64);
				ImGui::TextWrapped(name.c_str());
				ImGui::NextColumn();
				ImGui::PopID();
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
			return Type::NumTypes;
		}
		void AssetBrowser::dragAndDrop(const std::filesystem::path& path) const
		{
			if (ImGui::BeginDragDropSource())
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
		
	}
}