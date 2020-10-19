#pragma once
#include <XYZ.h>




namespace XYZ {
	class ProjectBrowserPanel
	{
	public:
		ProjectBrowserPanel();

		bool OnInGuiRender();
		void OnEvent(Event& event);

	private:
		void deleteTemplate();

		bool onKeyPress(KeyPressedEvent& event);
	private:
		InGuiWindow* m_Window;
		InGuiWindow* m_DeleteDialogWindow;
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

		std::string m_SelectedFile;
		int32_t m_SelectedFileIndex = sc_InvalidIndex;

		uint32_t m_PathLength;
		uint32_t m_DirectoryPathLength;
		char m_ProjectPath[260]; // This is windows max path value

		bool m_PopupEnabled = false;
		bool m_NewOpen = false;
		bool m_DeleteDialog = false;

		glm::vec2 m_PopupPosition = glm::vec2(0);
	};
}