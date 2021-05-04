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
		virtual ~EditorLayer();

		virtual void OnAttach() override;
		virtual void OnDetach() override;
		virtual void OnUpdate(Timestep ts) override;
		virtual void OnEvent(Event& event) override;

	private:
		bool onMouseButtonPress(MouseButtonPressEvent& event);
		bool onMouseButtonRelease(MouseButtonReleaseEvent& event);
		bool onWindowResize(WindowResizeEvent& event);
		bool onKeyPress(KeyPressedEvent& event);
		bool onKeyRelease(KeyReleasedEvent& event);

	private:
		Ref<Scene> m_Scene;
		Editor::EditorCamera m_EditorCamera;
		Editor::SceneHierarchyPanel m_SceneHierarchy;
		Editor::InspectorPanel m_Inspector;
		Editor::MainPanel m_Main;
		Editor::SkinningEditor m_SkinningEditor;
		Editor::AnimationEditor m_AnimationEditor;
	private:	
		SceneEntity m_SelectedEntity;
		SceneEntity m_TestEntity;

		glm::vec3 m_Position = { 0,0,0 };
		glm::vec3 m_Rotation = { 0,0,0 };
	};
}