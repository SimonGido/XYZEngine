#pragma once

#include "XYZ/Scene/Scene.h"
#include "XYZ/Scene/SceneEntity.h"
#include "XYZ/Event/ApplicationEvent.h"
#include "XYZ/Event/InputEvent.h"
#include "XYZ/Renderer/Texture.h"
#include "XYZ/Utils/Math/Ray.h"
#include "XYZ/Scene/SceneEntity.h"

#include "Editor/EditorPanel.h"
#include "Editor/EditorCamera.h"

namespace XYZ {
	namespace Editor {

		class ScenePanel : public EditorPanel
		{
		public:
			ScenePanel(std::string name);
			~ScenePanel();
	
			virtual void OnImGuiRender(bool &open) override;
			virtual void OnUpdate(Timestep ts) override;
			virtual bool OnEvent(Event& event) override;

			virtual void SetSceneContext(const Ref<Scene>& context);

			void SetSceneRenderer(const Ref<SceneRenderer>& sceneRenderer);

			EditorCamera& GetEditorCamera() { return m_EditorCamera; }
			
		private:
			bool onKeyPressed(KeyPressedEvent& e);

			std::pair<glm::vec3, glm::vec3> castRay(float mx, float my) const;
			std::pair<float, float>		    getMouseViewportSpace()		const;
			std::deque<SceneEntity>		    findSelection(const Ray& ray);

			bool playBar();
			bool toolsBar();

			void handlePanelResize(const glm::vec2& newSize);
			void handleSelection(const glm::vec2& mousePosition);
			void handleEntityTransform(SceneEntity entity);

			void acceptDragAndDrop();
		private:
			Ref<SceneRenderer>			m_SceneRenderer;
			Ref<Scene>					m_Context;
			glm::vec2				    m_ViewportSize;
			std::array<glm::vec2, 2>	m_ViewportBounds{};

	
			glm::vec2 m_ButtonSize;

			bool m_ViewportFocused;
			bool m_ViewportHovered;

			EditorCamera m_EditorCamera;


			std::deque<SceneEntity> m_Selection;
			uint32_t				m_SelectionIndex;
			int						m_GizmoType;

			static constexpr int    sc_InvalidGizmoType = -1;
		};
	}
}