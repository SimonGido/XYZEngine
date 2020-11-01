#pragma once

#include <XYZ.h>

#include "Panels/SceneHierarchyPanel.h"
#include "Panels/InspectorPanel.h"
#include "Panels/AnimatorPanel.h"
#include "Panels/SpriteEditorPanel.h"
#include "Panels/ProjectBrowserPanel.h"
#include "Panels/ScenePanel.h"
#include "Inspectable/InspectableSprite.h"
#include "Inspectable/InspectableEntity.h"
#include "Inspectable/InspectableAnimator.h"

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
		bool onMouseButtonPress(MouseButtonPressEvent& event);
		bool onMouseButtonRelease(MouseButtonReleaseEvent& event);
		
		bool onDeselected(DeselectedEvent& event);
		bool onEntitySelected(EntitySelectedEvent& event);
		bool onAnimatorSelected(AnimatorSelectedEvent& event);
		bool onSpriteSelected(SpriteSelectedEvent& event);
		ParticleVertex* m_Vertices;
		ParticleInformation* m_Data;
		ParticleComponent* m_Particle;
	private:
		enum
		{
			FOLDER = InGuiRenderConfiguration::DOCKSPACE + 1,
			SPRITE,
			TEXTURE,
			MATERIAL,
			SHADER,
			LOGO,
			PLAY,
			PAUSE
		};

		SceneHierarchyPanel m_SceneHierarchyPanel;
		SpriteEditorPanel m_SpriteEditorPanel;
		ProjectBrowserPanel m_ProjectBrowserPanel;
		AnimatorPanel m_AnimatorPanel;
		InspectorPanel m_InspectorPanel;
		ScenePanel m_ScenePanel;

	
		InspectableEntity m_InspectableEntity;
		InspectableAnimator m_InspectableAnimator;
		InspectableSprite m_InspectableSprite;

		bool m_Dragging = false;

		Ref<Scene> m_Scene;
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


		bool m_MenuOpen = false;
		bool m_Selecting = false;
		bool m_LeftPanel = false;
	
		Ref<Material> m_Material;
		Ref<Texture2D> m_CharacterTexture;

		Ref<SubTexture2D> m_CharacterSubTexture;
		Ref<SubTexture2D> m_CharacterSubTexture2;
		Ref<SubTexture2D> m_CharacterSubTexture3;


		void* m_ProhibitedCursor = nullptr;
	};
}