#include "stdafx.h"
#include "AssetBrowser.h"

#include "XYZ/Utils/StringUtils.h"
#include "XYZ/Asset/AssetManager.h"

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
			m_PathLength(6)
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
		
			m_Colors[ArrowColor]	    = glm::vec4(0.3f, 0.3f, 0.3f, 1.0f);
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
				size_t slashPos = FindLastOf(m_Path, '/');

				bool backArrowAvailable = slashPos != std::string::npos;
				bool frontArrowAvailable = !m_DirectoriesVisited.empty();

				ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
				glm::vec4 arrowColor = backArrowAvailable ? m_Colors[ArrowColor] : m_Colors[ArrowInvalidColor];
				ImGui::PushID("BackArrow");
				if (ImGui::ImageButton((void*)m_Texture->GetRendererID(), { m_ArrowSize.x, m_ArrowSize.y }, { m_TexCoords[Arrow].z, m_TexCoords[Arrow].y }, { m_TexCoords[Arrow].x, m_TexCoords[Arrow].w }, -1, {}, { arrowColor.r, arrowColor.g, arrowColor.b, arrowColor.a })
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
				if (ImGui::ImageButton((void*)m_Texture->GetRendererID(), { m_ArrowSize.x, m_ArrowSize.y }, { m_TexCoords[Arrow].x, m_TexCoords[Arrow].y }, { m_TexCoords[Arrow].z, m_TexCoords[Arrow].w }, -1, {}, { arrowColor.r, arrowColor.g, arrowColor.b, arrowColor.a })
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

			//if (InGui::Begin("Assets", 
			//	  InGuiWindowStyleFlags::PanelEnabled
			//	| InGuiWindowStyleFlags::ScrollEnabled
			//	| InGuiWindowStyleFlags::LabelEnabled
			//	| InGuiWindowStyleFlags::DockingEnabled
			//	| InGuiWindowStyleFlags::FrameEnabled
			//))
			//{
			//	InGuiConfig& config = InGui::GetContext().m_Config;
			//	m_Window = InGui::GetContext().m_FrameData.CurrentWindow;
			//	size_t slashPos = m_Path.find_last_of('/');
			//	
			//	m_Window->Axis = AxisPlacement::Horizontal;												
			//	m_Config.Begin(Config::BackArrow, Config::BackArrowHighlight);
			//	if (IS_SET(InGui::Image("########", glm::vec2(25.0f), config.SubTextures[InGuiConfig::LeftArrow]), InGui::Pressed))
			//	{
			//		if (slashPos != std::string::npos)
			//		{
			//			m_DirectoriesVisited.push_front(m_Path.substr(slashPos));
			//			m_Path.erase(m_Path.begin() + slashPos, m_Path.end());
			//		}
			//	}
			//	if (IS_SET(InGui::Image("#########", glm::vec2(25.0f), config.SubTextures[InGuiConfig::RightArrow]), InGui::Pressed))
			//	{
			//		if (!m_DirectoriesVisited.empty())
			//		{
			//			m_Path += m_DirectoriesVisited.front();
			//			m_DirectoriesVisited.pop_front();
			//		}
			//	}
			//	m_Config.End();
			//
			//	m_Window->Axis = AxisPlacement::Vertical;
			//	float stringPathWidth = m_Window->Size.x + m_Window->Position.x - m_Window->FrameData.CursorPos.x - config.WindowPadding.x;
			//	if (IS_SET(InGui::String("##", glm::vec2(stringPathWidth, 25.0f), m_Path), InGui::Pressed))
			//	{
			//		m_DirectoriesVisited.clear();
			//	}
			//
			//	m_Window->Axis = AxisPlacement::Horizontal;
			//	m_Window->FrameData.CursorPos.x = m_Window->Position.x + config.WindowPadding.x;
			//	m_Window->FrameData.Params.RowHeight = m_IconSize.y;
			//	
			//	processDirectory(m_Path);
			//	m_Config.End();
			//}
			//InGui::End();
		}
		void AssetBrowser::processDirectory(const std::string& path)
		{
			if (!std::filesystem::is_directory(path))
				return;

			for (auto it : std::filesystem::directory_iterator(path))
			{
				std::string name = Utils::GetFilename(it.path().string());
				ImGui::PushID(name.c_str());
				if (it.is_directory())
				{
					ImVec2 pos = ImGui::GetCursorPos();
					if (ImGui::ImageButton((void*)m_Texture->GetRendererID(), { m_IconSize.x, m_IconSize.y }, { m_TexCoords[Folder].x, m_TexCoords[Folder].y }, { m_TexCoords[Folder].z, m_TexCoords[Folder].w }))
					{
						std::string subPath = "/" + name;
						Add(m_Path, m_PathLength, subPath.c_str());
						m_PathLength += subPath.size();
						m_DirectoriesVisited.clear();
					}
					pos.x += m_IconSize.x + 15.0f;
					pos.y += m_IconSize.y / 2.0f;
					ImGui::SameLine();
					ImGui::SetCursorPos(pos);

					ImGui::Text(name.c_str());
				}
				else
				{
					AssetType type = AssetManager::GetAssetTypeFromExtension(Utils::GetExtension(name));
					switch (type)
					{
					case XYZ::AssetType::Scene:
						break;
					case XYZ::AssetType::Texture:
						break;
					case XYZ::AssetType::SubTexture:
						break;
					case XYZ::AssetType::Material:

						//InGui::Image(name.c_str(), m_IconSize, m_SubTextures[Material]);
						break;
					case XYZ::AssetType::Shader:

						//InGui::Image(name.c_str(), m_IconSize, m_SubTextures[Shader]);
						break;
					case XYZ::AssetType::Font:
						break;
					case XYZ::AssetType::Audio:
						break;
					case XYZ::AssetType::Script:

						//InGui::Image(name.c_str(), m_IconSize, m_SubTextures[Script]);
						break;
					case XYZ::AssetType::SkeletalMesh:
						break;
					case XYZ::AssetType::None:
						break;
					default:
						break;
					}
				}
				ImGui::PopID();
			}
		}
	}
}