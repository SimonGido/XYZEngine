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
		enum Editors
		{
			SceneHierarchy, SpriteEditor, AnimationEditor, NumEditors
		};
		bool m_EditorOpen[NumEditors];
		
		Ref<Scene>					m_Scene;
		Ref<SceneRenderer>			m_SceneRenderer;
		Ref<Renderer2D>				m_SceneRenderer2D;
		Ref<EditorRenderer>			m_EditorRenderer;

	private:	
		SceneEntity					m_SelectedEntity;
		Editor::ScenePanel			m_ScenePanel;
		Editor::SceneHierarchyPanel m_SceneHierarchyPanel;


		Ref<Texture2D>			 m_Texture;
	};
}