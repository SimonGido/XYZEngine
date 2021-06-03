#include "stdafx.h"
#include "AssetBrowser.h"

#include "XYZ/InGui/InGuiUtil.h"
#include "XYZ/Utils/StringUtils.h"
#include "XYZ/Asset/AssetManager.h"

#include <filesystem>

namespace XYZ {
	namespace Editor {
		AssetBrowser::AssetBrowser()
			:
			m_Window(nullptr),
			m_IconSize(50.0f),
			m_Path("Assets")
		{
			m_Texture = Texture2D::Create({}, "Assets/Textures/Gui/icons.png");
			float divisor = 4.0f;
			float width  = (float)m_Texture->GetWidth();
			float height = (float)m_Texture->GetHeight();
			glm::vec2 size = glm::vec2(width / divisor, height / divisor);
			m_SubTextures[Folder] = Ref<XYZ::SubTexture>::Create(m_Texture, glm::vec2(0, 3), size);
			m_SubTextures[Folder]->Upside();

			m_SubTextures[Shader] = Ref<XYZ::SubTexture>::Create(m_Texture, glm::vec2(1, 3), size);
			m_SubTextures[Shader]->Upside();

			m_SubTextures[Script] = Ref<XYZ::SubTexture>::Create(m_Texture, glm::vec2(3, 3), size);
			m_SubTextures[Script]->Upside();

			m_SubTextures[Material] = Ref<XYZ::SubTexture>::Create(m_Texture, glm::vec2(2, 3), size);
			m_SubTextures[Material]->Upside();
		}
		void AssetBrowser::OnUpdate()
		{
			if (InGui::Begin("Assets", 
				  InGuiWindowStyleFlags::PanelEnabled
				| InGuiWindowStyleFlags::ScrollEnabled
				| InGuiWindowStyleFlags::LabelEnabled
				| InGuiWindowStyleFlags::DockingEnabled
				| InGuiWindowStyleFlags::FrameEnabled
			))
			{
				InGuiConfig& config = InGui::GetContext().m_Config;
				m_Window = InGui::GetContext().m_FrameData.CurrentWindow;
				size_t slashPos = m_Path.find_last_of('/');
				
				m_Window->Axis = AxisPlacement::Horizontal;												
				m_Config.Begin(Config::BackArrow, Config::BackArrowHighlight);
				if (IS_SET(InGui::Image("########", glm::vec2(25.0f), config.SubTextures[InGuiConfig::LeftArrow]), InGui::Pressed))
				{
					if (slashPos != std::string::npos)
					{
						m_DirectoriesVisited.push_front(m_Path.substr(slashPos));
						m_Path.erase(m_Path.begin() + slashPos, m_Path.end());
					}
				}
				if (IS_SET(InGui::Image("#########", glm::vec2(25.0f), config.SubTextures[InGuiConfig::RightArrow]), InGui::Pressed))
				{
					if (!m_DirectoriesVisited.empty())
					{
						m_Path += m_DirectoriesVisited.front();
						m_DirectoriesVisited.pop_front();
					}
				}
				m_Config.End();

				m_Window->Axis = AxisPlacement::Vertical;
				float stringPathWidth = m_Window->Size.x + m_Window->Position.x - m_Window->FrameData.CursorPos.x - config.WindowPadding.x;
				if (IS_SET(InGui::String("##", glm::vec2(stringPathWidth, 25.0f), m_Path), InGui::Pressed))
				{
					m_DirectoriesVisited.clear();
				}

				m_Window->Axis = AxisPlacement::Horizontal;
				m_Window->FrameData.CursorPos.x = m_Window->Position.x + config.WindowPadding.x;
				m_Window->FrameData.Params.RowHeight = m_IconSize.y;
				
				processDirectory(m_Path);
				m_Config.End();
			}
			InGui::End();
		}
		void AssetBrowser::processDirectory(const std::string& path)
		{
			for (auto it : std::filesystem::directory_iterator(path))
			{
				std::string name = Utils::GetFilename(it.path().string());

				if (it.is_directory())
				{
					m_Config.Begin(Config::Folder, Config::FolderHighlight);
					if (IS_SET(InGui::Image(name.c_str(), m_IconSize, m_SubTextures[Folder]), InGui::Pressed))
					{
						m_Path += "/" + name;
					}
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
						m_Config.Begin(Config::Material, Config::MaterialHighlight);
						InGui::Image(name.c_str(), m_IconSize, m_SubTextures[Material]);
						break;
					case XYZ::AssetType::Shader:
						m_Config.Begin(Config::Shader, Config::ShaderHighlight);
						InGui::Image(name.c_str(), m_IconSize, m_SubTextures[Shader]);
						break;
					case XYZ::AssetType::Font:
						break;
					case XYZ::AssetType::Audio:
						break;
					case XYZ::AssetType::Script:
						m_Config.Begin(Config::Script, Config::ScriptHighlight);
						InGui::Image(name.c_str(), m_IconSize, m_SubTextures[Script]);
						break;
					case XYZ::AssetType::SkeletalMesh:
						break;
					case XYZ::AssetType::None:
						break;
					default:
						break;
					}
				}


				float textWidth = Util::CalculateTextSize(name.c_str(), InGui::GetContext().m_Config.Font).x;
				if (m_Window->FrameData.CursorPos.x + textWidth + m_IconSize.x
					  > m_Window->Position.x + m_Window->Size.x)
				{
					InGui::Separator();
				}
			}
		}
		AssetBrowser::Config::Config()
			:
			OldColor(0.0f),
			OldHighlight(0.0f),
			IsBegin(false)
		{
			for (size_t i = 0; i < Config::NumColors; i += 2)
			{
				Colors[i] = glm::vec4(0.1f, 0.3f, 0.4f, 1.0f);
				Colors[i + 1] = glm::vec4(0.3f, 0.4f, 0.9f, 1.0f);
			}
		}
		void AssetBrowser::Config::Begin(size_t colorID, size_t highlightID)
		{
			InGuiConfig& config = InGui::GetContext().m_Config;
			if (!IsBegin)
			{
				OldColor = config.Colors[InGuiConfig::ImageColor];
				OldHighlight = config.Colors[InGuiConfig::ImageHighlight];
			}
			config.Colors[InGuiConfig::ImageColor] = Colors[colorID];
			config.Colors[InGuiConfig::ImageHighlight] = Colors[highlightID];
			IsBegin = true;
		}
		void AssetBrowser::Config::End()
		{
			InGuiConfig& config = InGui::GetContext().m_Config;
			config.Colors[InGuiConfig::ImageColor] = OldColor;
			config.Colors[InGuiConfig::ImageHighlight] = OldHighlight;
			IsBegin = false;
		}
	}
}