#pragma once
#include <XYZ.h>

#include "Panel.h"

#include <filesystem>


namespace XYZ {
	class ProjectBrowserPanel : public Panel
	{
	public:
		ProjectBrowserPanel(uint32_t id);

		virtual void OnInGuiRender() override;
		virtual void OnUpdate(Timestep ts) override;
		virtual void OnEvent(Event& event) override;

	private:
		void handleFile(const char* name,const std::filesystem::directory_entry& entry, uint32_t counter, uint32_t subTextureIndex);
		bool onKeyPress(KeyPressedEvent& event);

	private: 
		
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