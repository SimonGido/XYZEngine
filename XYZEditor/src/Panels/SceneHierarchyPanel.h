#pragma once

#include <XYZ.h>

namespace XYZ {

	class SceneHierarchyPanel
	{
	public:
		SceneHierarchyPanel(uint32_t panelID);
		~SceneHierarchyPanel();

		void SetContext(Ref<Scene> context);


		void OnInGuiRender();
		void OnEvent(Event& event);
	private:
		void resizeEntities();

		bool onMouseButtonPress(MouseButtonPressEvent& event);
		bool onKeyPressed(KeyPressedEvent& event);

	private:
		uint32_t m_PanelID;
		Ref<Scene> m_Context;

		glm::vec2 m_DropdownPosition = glm::vec2(0.0f);
		bool m_DropdownOpen = false;
		bool* m_EntitiesOpen = nullptr;
		size_t m_CurrentSize = 0;
	};
}