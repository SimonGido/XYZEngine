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
		void displayStats();
		void gpuParticleExample(SceneEntity entity);
		void cpuParticleExample(SceneEntity entity);
		void animationExample(SceneEntity entity);

	private:
		enum Editors
		{
			SpriteEditor, AnimationEditor, NumEditors
		};
		bool m_EditorOpen[NumEditors];
		
		Ref<Scene> m_Scene;
		Editor::SceneHierarchyPanel m_SceneHierarchy;
		Editor::InspectorPanel	    m_Inspector;
		Editor::ScenePanel		    m_ScenePanel;
		Editor::AssetBrowser	    m_AssetBrowser;
		Editor::SkinningEditor	    m_SkinningEditor;
		Editor::SpriteEditor	    m_SpriteEditor;
		Editor::AnimationEditor	    m_AnimationEditor;

		Editor::SceneEntityInspectorContext m_SceneEntityInspectorContext;
		Editor::AssetInspectorContext	    m_AssetInspectorContext;
	private:	
		SceneEntity m_SelectedEntity;
		Timestep	m_Timestep;
	};
}