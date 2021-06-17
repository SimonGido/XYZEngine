#pragma once

#include "XYZ/Scene/Scene.h"
#include "XYZ/Scene/SceneEntity.h"
#include "XYZ/Event/ApplicationEvent.h"
#include "XYZ/Event/InputEvent.h"

namespace XYZ {
	namespace Editor {

		using SceneEntitySelectedCallback = std::function<void(SceneEntity)>;
		class ScenePanel
		{
		public:
			ScenePanel();

			void SetContext(const Ref<Scene>& context);
			void SetEntitySelectedCallback(const SceneEntitySelectedCallback& callback) { m_Callback = callback; }
	
			void OnUpdate(Timestep ts);
			void OnImGuiRender();

			EditorCamera& GetEditorCamera() { return m_EditorCamera; }
		private:
			std::pair<glm::vec3, glm::vec3> castRay(float mx, float my) const;
			std::pair<float, float> getMouseViewportSpace() const;

			void showSelection(SceneEntity entity);
			void handleSelection(const glm::vec2& mousePosition);
		private:
			SceneEntitySelectedCallback m_Callback;
			Ref<Scene> m_Context;
			glm::vec2 m_ViewportSize;
			glm::vec2 m_ViewportBounds[2];
			Ref<Texture> m_Texture;

			enum { PlayButton, StopButton, NumButtons};
			glm::vec4 m_ButtonTexCoords[NumButtons];
			glm::vec2 m_ButtonSize;

			bool m_ViewportFocused;
			bool m_ViewportHovered;

			EditorCamera m_EditorCamera;


			enum ModifyFlags
			{
				Move   = BIT(0),
				Rotate = BIT(1),
				Scale  = BIT(2),
				X	   = BIT(3),
				Y	   = BIT(4),
				Z	   = BIT(5)
			};

			
			std::deque<Entity> m_Selection;
			uint8_t			   m_ModifyFlags;
			float			   m_MoveSpeed;
			glm::vec2		   m_OldMousePosition;
		};
	}
}