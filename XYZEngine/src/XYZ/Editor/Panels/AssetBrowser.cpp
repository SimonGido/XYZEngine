#include "stdafx.h"
#include "AssetBrowser.h"

#include "XYZ/InGui/InGuiUtil.h"
#include "XYZ/Utils/StringUtils.h"

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
			m_SubTextures[Folder] = Ref<XYZ::SubTexture>::Create(m_Texture, glm::vec2(0, 3), glm::vec2(width / divisor, height / divisor));
			m_SubTextures[Folder]->Upside();
		}
		void AssetBrowser::OnUpdate()
		{
			if (InGui::Begin("Assets"))
			{
				const InGuiConfig& config = InGui::GetContext().m_Config;
				m_Window = InGui::GetContext().m_FrameData.CurrentWindow;
				size_t slashPos = m_Path.find_last_of('/');
				if (slashPos != std::string::npos)
				{
					m_Window->Axis = AxisPlacement::Horizontal;			
					
					if (IS_SET(InGui::Image("########", glm::vec2(25.0f), config.SubTextures[InGuiConfig::LeftArrow]), InGui::Pressed))
					{
						m_Path.erase(m_Path.begin() + slashPos, m_Path.end());
					}
					
				}
				m_Window->Axis = AxisPlacement::Vertical;
				float stringPathWidth = m_Window->Size.x + m_Window->Position.x - m_Window->FrameData.CursorPos.x - config.WindowPadding.x;
				InGui::String("##", glm::vec2(stringPathWidth, 25.0f), m_Path);

				m_Window->Axis = AxisPlacement::Horizontal;
				m_Window->FrameData.CursorPos.x = m_Window->Position.x + config.WindowPadding.x;
				m_Window->FrameData.Params.RowHeight = m_IconSize.y;
				processDirectory(m_Path);
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
					if (IS_SET(InGui::Image(name.c_str(), m_IconSize, m_SubTextures[Folder]), InGui::Pressed))
					{
						m_Path += "/" + name;
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
	}
}