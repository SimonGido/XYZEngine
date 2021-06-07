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
		virtual void OnImGuiRender() override;

	private:
		bool onMouseButtonPress(MouseButtonPressEvent& event);
		bool onMouseButtonRelease(MouseButtonReleaseEvent& event);
		bool onWindowResize(WindowResizeEvent& event);
		bool onKeyPress(KeyPressedEvent& event);
		bool onKeyRelease(KeyReleasedEvent& event);

	private:
		Ref<Scene> m_Scene;
		Editor::SceneHierarchyPanel m_SceneHierarchy;
		Editor::InspectorPanel m_Inspector;
		Editor::ScenePanel m_ScenePanel;
		Editor::AssetBrowser m_AssetBrowser;

		Editor::SceneEntityInspectorContext m_SceneEntityInspectorContext;
		Editor::AssetInspectorContext	    m_AssetInspectorContext;
	private:	
		SceneEntity m_SelectedEntity;

	};
}