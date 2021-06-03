#pragma once

#include "XYZ/Scene/Scene.h"
#include "XYZ/Scene/SceneEntity.h"
#include "XYZ/Event/ApplicationEvent.h"
#include "XYZ/Event/InputEvent.h"

namespace XYZ {
	namespace Editor {
		class ScenePanel
		{
		public:
			ScenePanel();

			void SetContext(Ref<Scene> context);

			void OnUpdate(Timestep ts);
			void OnImGuiRender();

			EditorCamera& GetEditorCamera() { return m_EditorCamera; }
		private:
			std::pair<glm::vec3, glm::vec3> castRay(float mx, float my) const;
			std::pair<float, float> getMouseViewportSpace() const;

		private:
			Ref<Scene> m_Context;
			glm::vec2 m_ViewportSize;
			glm::vec2 m_ViewportBounds[4];
			bool m_ViewportFocused;
			bool m_ViewportHovered;

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