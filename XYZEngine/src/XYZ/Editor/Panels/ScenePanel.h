#pragma once

#include "XYZ/Scene/Scene.h"
#include "XYZ/Scene/SceneEntity.h"
#include "XYZ/Event/ApplicationEvent.h"
#include "XYZ/Event/InputEvent.h"
#include "XYZ/InGui/InGui.h"

namespace XYZ {
	namespace Editor {
		class ScenePanel
		{
		public:
			ScenePanel();

			void SetContext(Ref<Scene> context);
			void SetSubTexture(Ref<SubTexture> subTexture);

			void OnUpdate(Timestep ts);
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
			Ref<Scene> m_Context;
			Ref<SubTexture> m_SceneSubTexture;
			glm::vec2 m_ViewportSize;
			EditorCamera m_EditorCamera;
			InGuiWindow* m_Window;

			enum ModifyFlags
			{
				Move = BIT(0),
				Rotate = BIT(1),
				Scale = BIT(2),
				X = BIT(3),
				Y = BIT(4),
				Z = BIT(5)
			};

			enum State
			{
				Play,
				Pause,
				NumStates
			};
			Ref<SubTexture> m_ButtonSubTextures[NumStates];
			uint32_t m_State = Play;

			SceneEntity m_SelectedEntity;
			uint8_t m_ModifyFlags = 0;
			float m_MoveSpeed = 100.0f;
			glm::vec2 m_OldMousePosition = glm::vec2(0.0f);
		};
	}
}