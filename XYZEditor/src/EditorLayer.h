#pragma once

#include <XYZ.h>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/matrix_decompose.hpp>
#include <glm/gtc/type_ptr.hpp>



namespace XYZ {
	
	class EditorLayer : public Layer
	{
	public:
		EditorLayer();
		virtual ~EditorLayer() override;

		virtual void OnAttach() override;
		virtual void OnDetach() override;
		virtual void OnUpdate(Timestep ts) override;
		virtual void OnEvent(Event& event) override;
		virtual void OnImGuiRender() override;

	private:
		bool onMouseButtonPress(MouseButtonPressEvent& event);
		bool onMouseButtonRelease(MouseButtonReleaseEvent& event);
		bool onWindowResize(WindowResizeEvent& event);
		bool onKeyPress(KeyPressedEvent& event);
		bool onKeyRelease(KeyReleasedEvent& event);

	private:
		void displayStats();

	private:
	
		Ref<Scene>					m_Scene;
		Ref<SceneRenderer>			m_SceneRenderer;
	
	private:	
		SceneEntity							m_SelectedEntity;
		Editor::EditorManager				m_EditorManager;
	};
}