#pragma once
#include <XYZ.h>



namespace XYZ {
	class ProjectBrowserPanel
	{
	public:
		ProjectBrowserPanel();

		bool OnInGuiRender();



	private:
		std::string m_ProjectPath;

		const uint32_t s_ProjectPanelID = 7;
	};
}