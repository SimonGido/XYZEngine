#include "stdafx.h"
#include "ProjectBrowserPanel.h"

#include <filesystem>

namespace XYZ {
	static glm::vec2 MouseToWorld(const glm::vec2& point, const glm::vec2& windowSize)
	{
		glm::vec2 offset = { windowSize.x / 2,windowSize.y / 2 };
		return { point.x - offset.x, offset.y - point.y };
	}
	static bool HasExtension(const std::string& path, const char* extension)
	{
		auto lastDot = path.rfind('.');
		auto count = path.size() - lastDot;
		
		std::string_view view(path.c_str() + lastDot + 1, count);

		if (!view.compare(0, view.size() - 1, extension))
			return true;
		return false;
	}

	static void CreateUnique(std::string& path)
	{
		uint32_t counter = 0;
		if (std::filesystem::exists(path))
		{
			std::string numStr = std::to_string(counter);
			uint32_t digits = numStr.size();
			path += numStr;
			while (std::filesystem::exists(path))
			{
				path.erase(path.end() - digits);
				numStr = std::to_string(counter++);
				digits = numStr.size();
				path += numStr;
			}
		}
		std::filesystem::create_directory(path);
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
		m_Window = InGui::GetWindow(s_ProjectPanelID);
		m_Window->Flags |= InGuiWindowFlag::ForceNewLine;

		auto& renderConfig = InGui::GetRenderConfiguration();
		renderConfig.SubTexture[FOLDER] = Ref<SubTexture2D>::Create(renderConfig.InTexture, glm::vec2(2, 1), glm::vec2(renderConfig.InTexture->GetWidth() / 8, renderConfig.InTexture->GetHeight() / 4));
		renderConfig.SubTexture[SPRITE] = Ref<SubTexture2D>::Create(renderConfig.InTexture, glm::vec2(3, 1), glm::vec2(renderConfig.InTexture->GetWidth() / 8, renderConfig.InTexture->GetHeight() / 4));
		renderConfig.SubTexture[TEXTURE] = Ref<SubTexture2D>::Create(renderConfig.InTexture, glm::vec2(2, 0), glm::vec2(renderConfig.InTexture->GetWidth() / 8, renderConfig.InTexture->GetHeight() / 4));
		renderConfig.SubTexture[MATERIAL] = Ref<SubTexture2D>::Create(renderConfig.InTexture, glm::vec2(3, 0), glm::vec2(renderConfig.InTexture->GetWidth() / 8, renderConfig.InTexture->GetHeight() / 4));
		renderConfig.SubTexture[SHADER] = Ref<SubTexture2D>::Create(renderConfig.InTexture, glm::vec2(4, 0), glm::vec2(renderConfig.InTexture->GetWidth() / 8, renderConfig.InTexture->GetHeight() / 4));
		renderConfig.SubTexture[BACK_ARROW] = Ref<SubTexture2D>::Create(renderConfig.InTexture, glm::vec2(3, 2), glm::vec2(renderConfig.InTexture->GetWidth() / 8, renderConfig.InTexture->GetHeight() / 4));
	}
	bool ProjectBrowserPanel::OnInGuiRender()
	{
		bool active = false;
		if (InGui::Begin(s_ProjectPanelID, "Project", { -200,-200 }, { 300,300 }))
		{
			auto& renderConfig = InGui::GetRenderConfiguration();
			active = true;
			

			if (InGui::Icon(m_ProjectPath, {}, { 30,30 }, renderConfig.SubTexture[BACK_ARROW], renderConfig.TextureID))
			{
				while (m_ProjectPath[m_PathLength + m_DirectoryPathLength] != '\\' && m_DirectoryPathLength)
					m_DirectoryPathLength--;	
		
				m_ProjectPath[m_PathLength + m_DirectoryPathLength] = '\0';
			}
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
				else if (HasExtension(entry.path().u8string(), "glsl"))
				{
					InGui::Icon(path.c_str(), {}, { 50,50 }, renderConfig.SubTexture[SHADER], renderConfig.TextureID);
				}
			}

			if (m_PopupEnabled)
			{
				m_Window->Flags &= ~InGuiWindowFlag::AutoPosition;
				if (InGui::BeginPopup("Create", m_PopupPosition, glm::vec2{ 150,25 }, m_PopupEnabled))
				{		
					if (InGui::PopupItem("New Folder", { 150,25 }))
					{
						std::string tmpDir = m_ProjectPath;
						tmpDir += "\\New Folder";
						CreateUnique(tmpDir);
						m_PopupEnabled = false;
					}
					else if (InGui::PopupItem("New Material", { 150,25 }))
					{
						std::string tmpDir = m_ProjectPath;
						tmpDir += "\\New Material";
						CreateUnique(tmpDir);
						m_PopupEnabled = false;
					}		
				}
				InGui::EndPopup();
				InGui::Separator();
				m_Window->Flags |= InGuiWindowFlag::AutoPosition;
			}


			if (m_Window->Flags & InGuiWindowFlag::RightClicked)
			{
				auto [width, height] = Input::GetWindowSize();
				auto [mx, my] = Input::GetMousePosition();

				m_PopupEnabled = !m_PopupEnabled;
				m_PopupPosition = MouseToWorld({ mx,my }, { width,height });
			}
			else if (m_Window->Flags & InGuiWindowFlag::LeftClicked)
			{
				m_PopupEnabled = false;
			}
		}	
		InGui::End();

		return active;
	}
	
}