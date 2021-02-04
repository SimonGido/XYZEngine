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
		bool onKeyPress(KeyPressedEvent& event);
		bool onKeyRelease(KeyReleasedEvent& event);

		std::pair<glm::vec3, glm::vec3> castRay(float mx, float my) const;
		std::pair<float, float> getMouseViewportSpace() const;
	private:
		uint32_t m_PanelID;
		Ref<Scene> m_Context;
		Ref<SubTexture> m_SubTexture;
		glm::vec2 m_ViewportSize;

		EditorCamera m_EditorCamera;


		enum ModifyFlags
		{
			Move = BIT(0),
			Rotate = BIT(1),
			Scale = BIT(2),
			X = BIT(3),
			Y = BIT(4),
			Z = BIT(5)
		};
		uint32_t m_SelectedEntity = NULL_ENTITY;
		uint8_t m_ModifyFlags = 0;
		float m_MoveSpeed = 100.0f;
		glm::vec2 m_OldMousePosition = glm::vec2(0.0f);
	};
}