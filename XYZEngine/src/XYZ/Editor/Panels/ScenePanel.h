#pragma once

#include "XYZ/Scene/Scene.h"
#include "XYZ/Scene/SceneEntity.h"
#include "XYZ/Event/ApplicationEvent.h"
#include "XYZ/Event/InputEvent.h"
#include "XYZ/Renderer/Texture.h"
#include "XYZ/Utils/Math/Ray.h"
#include "XYZ/Scene/SceneEntity.h"

namespace XYZ {
	namespace Editor {

		using SceneEntitySelectedCallback = std::function<void(SceneEntity)>;
		class ScenePanel
		{
		public:
			ScenePanel();
			~ScenePanel();

			void SetContext(const Ref<Scene>& context);
			void SetEntitySelectedCallback(const SceneEntitySelectedCallback& callback) { m_Callback = callback; }
	
			void OnUpdate(Timestep ts);
			void OnImGuiRender(const Ref<Image2D>& finalImage);

			void OnEvent(Event& event);

			EditorCamera& GetEditorCamera() { return m_EditorCamera; }
		private:
			bool onKeyPressed(KeyPressedEvent& e);

			std::pair<glm::vec3, glm::vec3> castRay(float mx, float my) const;
			std::pair<float, float>		    getMouseViewportSpace() const;
			std::deque<SceneEntity>		    getSelection(const Ray& ray);

			void handlePanelResize(const glm::vec2& newSize);
			void handleSelection(const glm::vec2& mousePosition);
			void handleEntityTransform(SceneEntity entity);
		private:
			SceneEntitySelectedCallback m_Callback;
			Ref<Scene>					m_Context;
			glm::vec2				    m_ViewportSize;
			glm::vec2				    m_ViewportBounds[2];
			Ref<Texture2D>				m_Texture;

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

			std::deque<SceneEntity> m_Selection;
			uint32_t				m_SelectionIndex;
			uint8_t					m_ModifyFlags;
			float					m_MoveSpeed;
			glm::vec2				m_OldMousePosition;
			int						m_GizmoType;
		};
	}
}