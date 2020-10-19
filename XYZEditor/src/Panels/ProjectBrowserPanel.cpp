#include "stdafx.h"
#include "ProjectBrowserPanel.h"

#include "Panel.h"

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

	static void CreateUniqueFolder(std::string& path)
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
	static void CreateUniqueFile(std::string& path, const char* extension)
	{
		uint32_t counter = 0;
		if (std::filesystem::exists(path + "." + extension))
		{
			std::string numStr = std::to_string(counter);
			uint32_t digits = numStr.size();
			path += numStr;
			while (std::filesystem::exists(path + "." + extension))
			{
				path.erase(path.end() - digits);
				numStr = std::to_string(counter++);
				digits = numStr.size();
				path += numStr;
			}
		}
		path += ".";
		path += extension;

		std::ofstream ofs(path);
		ofs << "Test";
		ofs.close();
	}

	void ProjectBrowserPanel::deleteTemplate()
	{
		auto& renderConfig = InGui::GetRenderConfiguration();
		m_DeleteDialogWindow->Position = { -150,-60 };
		m_DeleteDialogWindow->Size = { 300,120 };
		if (InGui::Begin(PanelID::DeletePanel, "Delete selected file?", { -150,-60 }, { 300,120 }))
		{
			if (InGui::Button("Delete", { 75,40 }))
			{
				std::filesystem::remove(m_SelectedFile);
				m_SelectedFile.clear();
				m_SelectedFileIndex = sc_InvalidIndex;
				m_DeleteDialog = false;
				m_Window->Flags |= InGuiWindowFlag::Modified;
			}		
			if (InGui::Button("Cancel", { 75,40 }))
			{
				m_DeleteDialog = false;
			}		
			InGui::Icon({}, { 100,100 }, renderConfig.SubTexture[LOGO], renderConfig.TextureID);
		}
		// Prevent user from clicking anything else before choosing some option
		InGui::ResolveLeftClick();
		InGui::ResolveRightClick();
		InGui::End();
	}

	

	ProjectBrowserPanel::ProjectBrowserPanel()
	{
		m_PathLength = 0;
		m_DirectoryPathLength = 0;
		for (auto c : std::filesystem::current_path().u8string())
			m_ProjectPath[m_PathLength++] = c;
		m_ProjectPath[m_PathLength] = '\0';
		InGui::Begin(PanelID::DeletePanel, "Delete selected file?", { -150,-60 }, { 300,120 });
		InGui::End();
		m_DeleteDialogWindow = InGui::GetWindow(PanelID::DeletePanel);
		m_DeleteDialogWindow->Flags &= ~InGuiWindowFlag::Dockable;
		m_DeleteDialogWindow->Flags |= InGuiWindowFlag::ForceNewLine;
		InGui::Begin(PanelID::ProjectBrowser, "Project", { -200,-200 }, { 300,300 });
		InGui::End();
		m_Window = InGui::GetWindow(PanelID::ProjectBrowser);
		m_Window->Flags |= InGuiWindowFlag::ForceNewLine;

		auto& renderConfig = InGui::GetRenderConfiguration();
		float divisor = 4.0f;
		renderConfig.SubTexture[FOLDER] =   Ref<SubTexture2D>::Create(renderConfig.InTexture, glm::vec2(0, 3), glm::vec2(renderConfig.InTexture->GetWidth() / divisor, renderConfig.InTexture->GetHeight() / divisor));
		renderConfig.SubTexture[SPRITE] =   Ref<SubTexture2D>::Create(renderConfig.InTexture, glm::vec2(0, 2), glm::vec2(renderConfig.InTexture->GetWidth() / divisor, renderConfig.InTexture->GetHeight() / divisor));
		renderConfig.SubTexture[TEXTURE] =  Ref<SubTexture2D>::Create(renderConfig.InTexture, glm::vec2(2, 3), glm::vec2(renderConfig.InTexture->GetWidth() / divisor, renderConfig.InTexture->GetHeight() / divisor));
		renderConfig.SubTexture[MATERIAL] = Ref<SubTexture2D>::Create(renderConfig.InTexture, glm::vec2(3, 3), glm::vec2(renderConfig.InTexture->GetWidth() / divisor, renderConfig.InTexture->GetHeight() / divisor));
		renderConfig.SubTexture[SHADER] =   Ref<SubTexture2D>::Create(renderConfig.InTexture, glm::vec2(1, 3), glm::vec2(renderConfig.InTexture->GetWidth() / divisor, renderConfig.InTexture->GetHeight() / divisor));
		renderConfig.SubTexture[LOGO] =		Ref<SubTexture2D>::Create(renderConfig.InTexture, glm::vec2(1, 0), glm::vec2(renderConfig.InTexture->GetWidth() / (divisor / 2), renderConfig.InTexture->GetHeight() / (divisor / 2)));
	}
	bool ProjectBrowserPanel::OnInGuiRender()
	{
		bool active = false;

		if (m_DeleteDialog)
			deleteTemplate();

		if (InGui::Begin(PanelID::ProjectBrowser, "Project", { -200,-200 }, { 300,300 }))
		{
			auto& renderConfig = InGui::GetRenderConfiguration();
			active = true;
			

			if (InGui::Icon(m_ProjectPath, {}, { 30,30 }, renderConfig.SubTexture[InGuiRenderConfiguration::LEFT_ARROW], renderConfig.TextureID))
			{
				while (m_ProjectPath[m_PathLength + m_DirectoryPathLength] != '\\' && m_DirectoryPathLength)
					m_DirectoryPathLength--;	
		
				m_ProjectPath[m_PathLength + m_DirectoryPathLength] = '\0';
			}
			InGui::Separator();

					
			uint32_t offset = m_PathLength + m_DirectoryPathLength;
			uint32_t counter = 0;
			for (const auto& entry : std::filesystem::directory_iterator(m_ProjectPath))
			{	
				size_t count = entry.path().u8string().size() - offset - 1;
 				std::string path(entry.path().u8string().c_str() + offset + 1, count);

				bool highlight = false;
				if (m_SelectedFileIndex == counter)
					highlight = true;

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
					if (InGui::Icon(path.c_str(), {}, { 50,50 }, renderConfig.SubTexture[SPRITE], renderConfig.TextureID, highlight))
					{
						m_SelectedFile = entry.path().u8string();
						m_SelectedFileIndex = counter;
					}
				}
				else if (HasExtension(entry.path().u8string(), "mat"))
				{
					if (InGui::Icon(path.c_str(), {}, { 50,50 }, renderConfig.SubTexture[MATERIAL], renderConfig.TextureID, highlight))
					{
						m_SelectedFile = entry.path().u8string();
						m_SelectedFileIndex = counter;
					}
				}
				else if (HasExtension(entry.path().u8string(), "png"))
				{
					if (InGui::Icon(path.c_str(), {}, { 50,50 }, renderConfig.SubTexture[TEXTURE], renderConfig.TextureID, highlight))
					{
						m_SelectedFile = entry.path().u8string();
						m_SelectedFileIndex = counter;
					}
				}
				else if (HasExtension(entry.path().u8string(), "glsl"))
				{
					if (InGui::Icon(path.c_str(), {}, { 50,50 }, renderConfig.SubTexture[SHADER], renderConfig.TextureID, highlight))
					{
						m_SelectedFile = entry.path().u8string();
						m_SelectedFileIndex = counter;
					}
				}
				if ((m_Window->Flags & InGuiWindowFlag::Hoovered) && InGui::ResolveRightClick())
				{
					auto [width, height] = Input::GetWindowSize();
					auto [mx, my] = Input::GetMousePosition();

					m_SelectedFile.clear();
					m_SelectedFileIndex = sc_InvalidIndex;
					m_PopupEnabled = !m_PopupEnabled;
					m_PopupPosition = MouseToWorld({ mx,my }, { width,height });
				}
				
				counter++;
			}
		
			if (m_PopupEnabled)
			{
				m_Window->Flags &= ~InGuiWindowFlag::AutoPosition;
				if (InGui::BeginPopup("New", m_PopupPosition, glm::vec2{ 150,25 }, m_PopupEnabled))
				{
					if (InGui::PopupExpandItem("File -->", m_NewOpen))
					{
						if (InGui::PopupItem("New Folder"))
						{	
							std::string tmpDir = m_ProjectPath;
							tmpDir += "\\New Folder";
							CreateUniqueFolder(tmpDir);
							m_PopupEnabled = false;
							m_NewOpen = false;
						}
						else if (InGui::PopupItem("New Material"))
						{
							std::string tmpDir = m_ProjectPath;
							tmpDir += "\\New Material";
							CreateUniqueFile(tmpDir, "mat");
							m_PopupEnabled = false;
							m_NewOpen = false;
						}
						else if (InGui::PopupItem("New Shader"))
						{
							std::string tmpDir = m_ProjectPath;
							tmpDir += "\\New Shader";
							CreateUniqueFile(tmpDir, "glsl");
							m_PopupEnabled = false;
							m_NewOpen = false;
						}
						else if (InGui::PopupItem("New Subtexture"))
						{
							std::string tmpDir = m_ProjectPath;
							tmpDir += "\\New Subtexture";
							CreateUniqueFile(tmpDir, "subtex");
							m_PopupEnabled = false;
							m_NewOpen = false;
						}			
						
					}
					InGui::PopupExpandEnd();
				
					if (InGui::PopupItem("Test"))
					{
						m_PopupEnabled = false;
						m_NewOpen = false;
					}
					if (InGui::PopupItem("Test"))
					{
						m_PopupEnabled = false;
						m_NewOpen = false;
					}

					
				}
				
				InGui::EndPopup();
				InGui::Separator();
				m_Window->Flags |= InGuiWindowFlag::AutoPosition;
			}
			if (InGui::ResolveLeftClick(false))
			{
				m_SelectedFile.clear();
				m_SelectedFileIndex = sc_InvalidIndex;
				m_PopupEnabled = false;
				m_NewOpen = false;
			}
		}	
		InGui::End();

		return active;
	}

	void ProjectBrowserPanel::OnEvent(Event& event)
	{
		EventDispatcher dispatcher(event);
		dispatcher.Dispatch<KeyPressedEvent>(Hook(&ProjectBrowserPanel::onKeyPress, this));
	}
	bool ProjectBrowserPanel::onKeyPress(KeyPressedEvent& event)
	{
		if (event.IsKeyPressed(KeyCode::XYZ_KEY_DELETE))
		{
			if (!m_SelectedFile.empty())
			{
				m_DeleteDialog = true;
				m_DeleteDialogWindow->Flags |= InGuiWindowFlag::Modified;
			}
		}
		return false;
	}
}