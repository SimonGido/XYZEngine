#pragma once
#include <XYZ.h>

#include <filesystem>


namespace XYZ {
	class ProjectBrowserPanel
	{
	public:
		ProjectBrowserPanel();

		bool OnInGuiRender();
		void OnEvent(Event& event);

		int32_t GetSelectedFileIndex() const { return m_SelectedFileIndex; }
		const std::string& GetSelectedFilePath() const { return m_SelectedFile; }
		std::string GetSelectedFileFullPath() const;
	private:
		void handleFile(const char* name,const std::filesystem::directory_entry& entry, uint32_t counter, uint32_t subTextureIndex);
		bool onKeyPress(KeyPressedEvent& event);

	private: 
		InGuiWindow* m_Window;
		enum
		{
			FOLDER = InGuiRenderConfiguration::DOCKSPACE + 1,
			SPRITE,
			TEXTURE,
			MATERIAL,
			SHADER,
			LOGO
		};
		static constexpr int32_t sc_InvalidIndex = -1;

		std::string m_DeleteSelectedFile;
		std::string m_SelectedFile;
		int32_t m_SelectedFileIndex = sc_InvalidIndex;

		uint32_t m_PathLength;
		uint32_t m_DirectoryPathLength;
		char m_ProjectPath[260]; // This is windows max path value

		bool m_PopupEnabled = false;
		bool m_DeletePopupEnabled = false;
		bool m_NewOpen = false;

		glm::vec2 m_PopupPosition = glm::vec2(0);
	};
}