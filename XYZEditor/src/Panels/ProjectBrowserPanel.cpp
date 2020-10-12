#include "stdafx.h"
#include "ProjectBrowserPanel.h"

#include <filesystem>

namespace XYZ {

	static bool HasExtension(const std::string& path, const char* extension)
	{
		auto lastDot = path.rfind('.');
		auto count = path.size() - lastDot;
		
		std::string_view view(path.c_str() + lastDot + 1, count);

		if (!view.compare(0, view.size() - 1, extension))
			return true;
		return false;
	}

	ProjectBrowserPanel::ProjectBrowserPanel()
	{
		m_PathLength = 0;
		m_DirectoryPathLength = 0;
		for (auto c : std::filesystem::current_path().u8string())
			m_ProjectPath[m_PathLength++] = c;
		m_ProjectPath[m_PathLength] = '\0';
		
		InGui::Begin(s_ProjectPanelID, "Project", { -200,-200 }, { 300,300 });
		InGui::End();
		InGui::GetWindow(s_ProjectPanelID)->Flags |= InGuiWindowFlag::ForceNewLine;

		auto& renderConfig = InGui::GetRenderConfiguration();
		renderConfig.SubTexture[FOLDER] = Ref<SubTexture2D>::Create(renderConfig.InTexture, glm::vec2(2, 1), glm::vec2(renderConfig.InTexture->GetWidth() / 4, renderConfig.InTexture->GetHeight() / 4));
		renderConfig.SubTexture[SPRITE] = Ref<SubTexture2D>::Create(renderConfig.InTexture, glm::vec2(3, 1), glm::vec2(renderConfig.InTexture->GetWidth() / 4, renderConfig.InTexture->GetHeight() / 4));
		renderConfig.SubTexture[TEXTURE] = Ref<SubTexture2D>::Create(renderConfig.InTexture, glm::vec2(2, 0), glm::vec2(renderConfig.InTexture->GetWidth() / 4, renderConfig.InTexture->GetHeight() / 4));
		renderConfig.SubTexture[MATERIAL] = Ref<SubTexture2D>::Create(renderConfig.InTexture, glm::vec2(3, 0), glm::vec2(renderConfig.InTexture->GetWidth() / 4, renderConfig.InTexture->GetHeight() / 4));
		renderConfig.SubTexture[BACK_ARROW] = Ref<SubTexture2D>::Create(renderConfig.InTexture, glm::vec2(3, 2), glm::vec2(renderConfig.InTexture->GetWidth() / 4, renderConfig.InTexture->GetHeight() / 4));
	}
	bool ProjectBrowserPanel::OnInGuiRender()
	{
		bool active = false;
		if (InGui::Begin(s_ProjectPanelID, "Project", { -200,-200 }, { 300,300 }))
		{
			auto& renderConfig = InGui::GetRenderConfiguration();
			active = true;
			

			if (InGui::Icon({}, { 50,25 }, renderConfig.SubTexture[BACK_ARROW], renderConfig.TextureID))
			{
				while (m_ProjectPath[m_PathLength + m_DirectoryPathLength] != '\\' && m_DirectoryPathLength)
					m_DirectoryPathLength--;	
		
				m_ProjectPath[m_PathLength + m_DirectoryPathLength] = '\0';
			}
			InGui::Text(m_ProjectPath, { 0.7f,0.7f }, { 0.7f,0.7f,0.7f,1.0f });
			InGui::Separator();


			uint32_t offset = m_PathLength + m_DirectoryPathLength;
			for (const auto& entry : std::filesystem::directory_iterator(m_ProjectPath))
			{	
				size_t count = entry.path().u8string().size() - offset - 1;
 				std::string path(entry.path().u8string().c_str() + offset + 1, count);
				if (entry.is_directory())
				{
					if (InGui::Icon(path.c_str(), {}, { 50,50 }, renderConfig.SubTexture[FOLDER], renderConfig.TextureID))
					{
						m_ProjectPath[offset++] = '\\';
						m_DirectoryPathLength++;
						memcpy(&m_ProjectPath[m_PathLength + m_DirectoryPathLength], path.c_str(), path.size());
						m_DirectoryPathLength += path.size();
						m_ProjectPath[m_PathLength + m_DirectoryPathLength] = '\0';
					}
				}
				else if (HasExtension(entry.path().u8string(), "subtex"))
				{
					InGui::Icon(path.c_str(), {}, { 50,50 }, renderConfig.SubTexture[SPRITE], renderConfig.TextureID);
				}
				else if (HasExtension(entry.path().u8string(), "mat"))
				{
					InGui::Icon(path.c_str(), {}, { 50,50 }, renderConfig.SubTexture[MATERIAL], renderConfig.TextureID);
				}
				else if (HasExtension(entry.path().u8string(), "png"))
				{
					InGui::Icon(path.c_str(), {}, { 50,50 }, renderConfig.SubTexture[TEXTURE], renderConfig.TextureID);
				}
			}
		}
		InGui::End();

		return active;
	}
	
}