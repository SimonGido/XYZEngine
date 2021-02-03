#pragma once

#include <XYZ.h>

namespace XYZ {

	class ScenePanel
	{
	public:
		ScenePanel(uint32_t panelID);

		void SetContext(Ref<Scene> context);
		void SetSubTexture(Ref<SubTexture> subTexture);

		void OnUpdate(Timestep ts);
		void OnInGuiRender();
		void OnEvent(Event& event);

		EditorCamera& GetEditorCamera() { return m_EditorCamera; }
	private:
		bool onWindowResize(WindowResizeEvent& event);
		bool onMouseButtonPress(MouseButtonPressEvent& event);

		std::pair<glm::vec3, glm::vec3> castRay(float mx, float my) const;
		std::pair<float, float> getMouseViewportSpace() const;
	private:
		uint32_t m_PanelID;
		Ref<Scene> m_Context;
		Ref<SubTexture> m_SubTexture;
		glm::vec2 m_ViewportSize;

		EditorCamera m_EditorCamera;
	};
}