#pragma once

#include <XYZ.h>

#include "Panels/SceneHierarchyPanel.h"
#include "Panels/InspectorPanel.h"
#include "Panels/GraphPanel.h"
#include "Panels/SpriteEditorPanel.h"
#include "Panels/ProjectBrowserPanel.h"

#include "InspectorLayout/SpriteEditorInspectorLayout.h"
#include "InspectorLayout/EntityInspectorLayout.h"
#include "InspectorLayout/AnimatorInspectorLayout.h"
#include "GraphLayout/AnimatorGraphLayout.h"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/matrix_decompose.hpp>
#include <glm/gtc/type_ptr.hpp>


// TODO: solve fragmentation
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
		virtual void OnInGuiRender(Timestep ts) override;

	private:
		bool onWindowResized(WindowResizeEvent& event);
		bool onMouseButtonPress(MouseButtonPressEvent& event);
		bool onMouseButtonRelease(MouseButtonReleaseEvent& event);
		bool onKeyPress(KeyPressedEvent& event);
		bool onKeyRelease(KeyReleasedEvent& event);

		void onResizeSceneWindow(const glm::vec2& size);

	private:
		enum
		{
			FOLDER = InGuiRenderConfiguration::DOCKSPACE + 1,
			SPRITE,
			TEXTURE,
			MATERIAL,
			SHADER,
			LOGO
		};

		InGuiWindow* m_SceneWindow = nullptr;
		SceneHierarchyPanel m_SceneHierarchyPanel;
		SpriteEditorPanel m_SpriteEditorPanel;
		ProjectBrowserPanel m_ProjectBrowserPanel;

		InspectorPanel m_InspectorPanel;
		EntityInspectorLayout m_EntityInspectorLayout;
		AnimatorInspectorLayout m_AnimatorInspectorLayout;
		SpriteEditorInspectorLayout m_SpriteEditorInspectorLayout;

		GraphPanel m_GraphPanel;
		AnimatorGraphLayout m_AnimatorGraphLayout;
		Graph m_NodeGraph;

		Entity m_SelectedEntity;
		glm::vec2 m_StartMousePos;
		bool m_ScalingEntity = false;
		bool m_MovingEntity = false;
		bool m_RotatingEntity = false;
		bool m_Dragging = false;

		TransformComponent* m_ModifiedTransform = nullptr;
		glm::vec3 m_ModifiedTranslation;
		glm::vec3 m_ModifiedRotation;
		glm::vec3 m_ModifiedScale;


		EditorCamera m_EditorCamera;
		Ref<Scene> m_Scene;
		Ref<RenderPass> m_RenderPass;
		Ref<FrameBuffer> m_FBO;
		AssetManager m_AssetManager;
		std::vector<Entity> m_StoredEntitiesWithScript;

	private:	
		Entity m_TestEntity;


		SpriteRenderer* m_SpriteRenderer;
		TransformComponent* m_Transform;
		AnimatorComponent* m_Animator;
		
		Ref<AnimationController> m_AnimationController;
		Ref<Animation> m_IdleAnimation;
		Ref<Animation> m_RunAnimation;

		glm::vec3 m_Position = { 0,0,0 };
		glm::vec3 m_Rotation = { 0,0,0 };


		bool m_ActiveWindow = false;
		bool m_MenuOpen = false;
		bool m_Selecting = false;

	
	
		Ref<Material> m_Material;
		Ref<Texture2D> m_CharacterTexture;

		Ref<SubTexture2D> m_CharacterSubTexture;
		Ref<SubTexture2D> m_CharacterSubTexture2;
		Ref<SubTexture2D> m_CharacterSubTexture3;



		void* m_ProhibitedCursor = nullptr;
	};
}