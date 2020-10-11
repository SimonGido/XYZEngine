#include "stdafx.h"
#include "ProjectBrowserPanel.h"

#include <filesystem>

namespace XYZ {
	ProjectBrowserPanel::ProjectBrowserPanel()
	{
		m_ProjectPath = std::filesystem::current_path().u8string();
		InGui::Begin(s_ProjectPanelID, "Project", { -200,-200 }, { 300,300 });
		InGui::End();
	}
	bool ProjectBrowserPanel::OnInGuiRender()
	{
		bool active = false;
		if (InGui::Begin(s_ProjectPanelID, "Project", { -200,-200 }, { 300,300 }))
		{
			active = true;


			size_t offset = m_ProjectPath.size() + 1;
			for (const auto& entry : std::filesystem::directory_iterator(m_ProjectPath))
			{
				size_t count = entry.path().u8string().size() - offset;
				std::string path(entry.path().u8string().c_str() + offset, count);
				InGui::Text(path.c_str(), { 0.7f,0.7f });
				InGui::Separator();
			}
		}
		InGui::End();

		return active;
	}
	
}