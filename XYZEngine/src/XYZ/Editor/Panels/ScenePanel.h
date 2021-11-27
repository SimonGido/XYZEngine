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

		class ScenePanel
		{
		public:
			ScenePanel();
			~ScenePanel();

			void SetContext(const Ref<Scene>& context);
		
			void OnUpdate(Timestep ts);
			void OnImGuiRender(const Ref<Image2D>& finalImage);

			void OnEvent(Event& event);

			EditorCamera& GetEditorCamera() { return m_EditorCamera; }
		private:
			struct UV
			{
				glm::vec2 UV0{}, UV1{};
			};

		private:
			bool onKeyPressed(KeyPressedEvent& e);

			std::pair<glm::vec3, glm::vec3> castRay(float mx, float my) const;
			std::pair<float, float>		    getMouseViewportSpace()		const;
			std::deque<SceneEntity>		    getSelection(const Ray& ray);

			void handlePanelResize(const glm::vec2& newSize);
			void handleSelection(const glm::vec2& mousePosition);
			void handleEntityTransform(SceneEntity entity);

		private:
			Ref<Scene>					m_Context;
			glm::vec2				    m_ViewportSize;
			std::array<glm::vec2, 2>	m_ViewportBounds;
			Ref<Texture2D>				m_Texture;

			enum { PlayButton, StopButton, NumButtons};
			
		

			UV		  m_ButtonUVs[NumButtons];
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