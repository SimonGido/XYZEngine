#pragma once
#include <XYZ.h>




namespace XYZ {
	class ProjectBrowserPanel
	{
	public:
		ProjectBrowserPanel();

		bool OnInGuiRender();
	private:
		InGuiWindow* m_Window;
		enum
		{
			FOLDER = InGuiRenderConfiguration::DOCKSPACE + 1,
			SPRITE,
			TEXTURE,
			MATERIAL,
			SHADER,
			BACK_ARROW
		};

		uint32_t m_PathLength;
		uint32_t m_DirectoryPathLength;
		char m_ProjectPath[260]; // This is windows max path value


		bool m_PopupEnabled = false;
		bool m_NewOpen = false;
		glm::vec2 m_PopupPosition = glm::vec2(0);

		const uint32_t s_ProjectPanelID = 7;
	};
}