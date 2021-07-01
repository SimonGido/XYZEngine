#include "stdafx.h"
#include "AssetBrowser.h"

#include "XYZ/Utils/StringUtils.h"
#include "XYZ/Asset/AssetManager.h"
#include "XYZ/Scene/Scene.h"
#include "XYZ/Renderer/Font.h"

#include <imgui.h>
#include <filesystem>

namespace XYZ {
	namespace Editor {

		static size_t FindLastOf(char* buffer, char c)
		{
			size_t length = strlen(buffer);
			size_t counter = length;
			while (counter != 0)
			{
				if (buffer[counter] == c)
					return counter;
				counter--;
			}
			return std::string::npos;
		}

		static char* SubString(const char* buffer, size_t start, size_t end)
		{
			size_t size = end - start;
			char* result = new char[size + 1];
			memcpy(result, &buffer[start], size);
			result[size] = '\0';
			return result;
		}

		static void Copy(char* buffer, const char* source)
		{
			size_t length = strlen(source);
			memcpy(buffer, source, length);
			buffer[length + 1] = '\0';
		}

		static void Add(char* buffer, size_t length, const char* source)
		{
			size_t newLength = strlen(source);
			memcpy(&buffer[length], source, newLength);
			buffer[length + newLength] = '\0';
		}

		static glm::vec4 CalculateTexCoords(const glm::vec2& coords, const glm::vec2& size, const glm::vec2& textureSize)
		{
			return {
				(coords.x * size.x) / textureSize.x,
				((coords.y + 1) * size.y) / textureSize.y,
				((coords.x + 1) * size.x) / textureSize.x,
				(coords.y * size.y) / textureSize.y,
			};
		}

		AssetBrowser::AssetBrowser()
			:
			m_IconSize(50.0f),
			m_ArrowSize(25.0f),
			m_Path("Assets"),
			m_PathLength(6),
			m_ViewportHovered(false),
			m_ViewportFocused(false)
		{
			m_Texture = Texture2D::Create({}, "Assets/Textures/Gui/icons.png");
			float divisor = 4.0f;
			float width  = (float)m_Texture->GetWidth();
			float height = (float)m_Texture->GetHeight();
			glm::vec2 size = glm::vec2(width / divisor, height / divisor);


			m_TexCoords[Arrow]    = CalculateTexCoords(glm::vec2(0, 2), size, { width, height });
			m_TexCoords[Folder]   = CalculateTexCoords(glm::vec2(0, 3), size, { width, height });
			m_TexCoords[Shader]   = CalculateTexCoords(glm::vec2(1, 3), size, { width, height });
			m_TexCoords[Script]   = CalculateTexCoords(glm::vec2(3, 3), size, { width, height });
			m_TexCoords[Material] = CalculateTexCoords(glm::vec2(2, 3), size, { width, height });
			m_TexCoords[Texture]  = CalculateTexCoords(glm::vec2(2, 2), size, { width, height });
			m_TexCoords[Mesh]	  = CalculateTexCoords(glm::vec2(1, 2), size, { width, height });

			m_Colors[ArrowColor]	    = glm::vec4(0.5f, 0.5f, 0.5f, 1.0f);
			m_Colors[ArrowInvalidColor] = glm::vec4(0.3f);
		}
		void AssetBrowser::SetPath(const std::string& path)
		{
			Copy(m_Path, path.c_str());
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
				size_t slashPos = FindLastOf(m_Path, '/');

				bool backArrowAvailable = slashPos != std::string::npos;
				bool frontArrowAvailable = !m_DirectoriesVisited.empty();

				ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
				glm::vec4 arrowColor = backArrowAvailable ? m_Colors[ArrowColor] : m_Colors[ArrowInvalidColor];
				ImGui::PushID("BackArrow");
				if (ImGui::ImageButton((void*)(uint64_t)m_Texture->GetRendererID(), { m_ArrowSize.x, m_ArrowSize.y }, { m_TexCoords[Arrow].z, m_TexCoords[Arrow].y }, { m_TexCoords[Arrow].x, m_TexCoords[Arrow].w }, -1, {}, { arrowColor.r, arrowColor.g, arrowColor.b, arrowColor.a })
				 && backArrowAvailable)
				{
					m_DirectoriesVisited.push_front(SubString(m_Path, slashPos, strlen(m_Path)));
					m_Path[slashPos] = '\0';
					m_PathLength = slashPos;
				}
				ImGui::PopID();
				ImGui::SameLine();
				
				arrowColor = frontArrowAvailable ? m_Colors[ArrowColor] : m_Colors[ArrowInvalidColor];
				ImGui::PushID("FrontArrow");
				if (ImGui::ImageButton((void*)(uint64_t)m_Texture->GetRendererID(), { m_ArrowSize.x, m_ArrowSize.y }, { m_TexCoords[Arrow].x, m_TexCoords[Arrow].y }, { m_TexCoords[Arrow].z, m_TexCoords[Arrow].w }, -1, {}, { arrowColor.r, arrowColor.g, arrowColor.b, arrowColor.a })
				&& frontArrowAvailable)
				{
					Add(m_Path, m_PathLength, m_DirectoriesVisited.front().c_str());
					m_PathLength += m_DirectoriesVisited.front().size();
					m_DirectoriesVisited.pop_front();
				}
				ImGui::PopID();
				ImGui::SameLine();
			
				if (ImGui::InputText("###", m_Path, _MAX_PATH))
				{
					m_PathLength = strlen(m_Path);
				}

				processDirectory(m_Path);
				ImGui::PopStyleColor();
			}
			ImGui::End();
		}
		Ref<Asset> AssetBrowser::GetSelectedAsset() const
		{
			if (!m_SelectedFile.empty())
			{
				std::string fullFilePath;
				fullFilePath.append(m_Path);
				fullFilePath.append("/" + m_SelectedFile);
				AssetType type = AssetManager::GetAssetTypeFromExtension(Utils::GetExtension(m_SelectedFile));
				switch (type)
				{
				case XYZ::AssetType::Scene:
					return AssetManager::GetAsset<XYZ::Scene>(AssetManager::GetAssetHandle(fullFilePath));
					break;
				case XYZ::AssetType::Texture:
					return AssetManager::GetAsset<XYZ::Texture>(AssetManager::GetAssetHandle(fullFilePath));
					break;
				case XYZ::AssetType::SubTexture:
					return AssetManager::GetAsset<XYZ::SubTexture>(AssetManager::GetAssetHandle(fullFilePath));
					break;
				case XYZ::AssetType::Material:
					return AssetManager::GetAsset<XYZ::Material>(AssetManager::GetAssetHandle(fullFilePath));
					break;
				case XYZ::AssetType::Shader:
					return AssetManager::GetAsset<XYZ::Shader>(AssetManager::GetAssetHandle(fullFilePath));
					break;
				case XYZ::AssetType::Font:
					return AssetManager::GetAsset<XYZ::Font>(AssetManager::GetAssetHandle(fullFilePath));
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
		void AssetBrowser::processDirectory(const std::string& path)
		{
			if (!std::filesystem::is_directory(path))
				return;

			float xOffset = 15.0f;
			ImVec2 cursorPos = ImGui::GetCursorPos();
			for (auto it : std::filesystem::directory_iterator(path))
			{
				std::string name = Utils::GetFilename(it.path().string());
				ImGui::PushID(name.c_str());
				if (cursorPos.x + m_IconSize.x + xOffset + ImGui::CalcTextSize(name.c_str()).x > ImGui::GetContentRegionAvail().x)
				{
					cursorPos.y += m_IconSize.y;
					cursorPos.x = ImGui::GetStyle().ItemSpacing.x;
				}
				ImGui::SetCursorPos(cursorPos);

				if (it.is_directory())
				{
					if (ImGui::ImageButton((void*)(uint64_t)m_Texture->GetRendererID(), { m_IconSize.x, m_IconSize.y }, { m_TexCoords[Folder].x, m_TexCoords[Folder].y }, { m_TexCoords[Folder].z, m_TexCoords[Folder].w }))
					{
						std::string subPath = "/" + name;
						Add(m_Path, m_PathLength, subPath.c_str());
						m_PathLength += subPath.size();
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
					if (ImGui::IsItemClicked(ImGuiMouseButton_Right))
					{
						ImGui::OpenPopup("LoadAsset");
					}
					if (ImGui::BeginPopup("LoadAsset"))
					{
						if (ImGui::MenuItem("Load Asset"))
						{
							AssetManager::LoadAsset(AssetManager::GetAssetHandle(fullPath(name)));
							ImGui::CloseCurrentPopup();
						}
						ImGui::EndPopup();
					}
				}

				cursorPos.x += m_IconSize.x + xOffset;
				cursorPos.y += m_IconSize.y / 2.0f;
				ImGui::SetCursorPos(cursorPos);
				ImGui::Text(name.c_str());
				cursorPos.x = ImGui::GetItemRectMax().x - ImGui::GetWindowPos().x;
				cursorPos.y -= m_IconSize.y / 2.0f;

				ImGui::PopID();
			}
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
			case XYZ::AssetType::None:
				return NumTypes;
				break;
			default:
				break;
			}
			return NumTypes;
		}
		std::string AssetBrowser::fullPath(const std::string& filename) const
		{
			std::string fullFilePath;
			fullFilePath.append(m_Path);
			fullFilePath.append("/" + filename);
			return fullFilePath;
		}
	}
}