#pragma once

#include <XYZ.h>

#include "Panels/SceneHierarchyPanel.h"


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
		virtual void OnInGuiRender() override;

	private:
		bool onMouseButtonPress(MouseButtonPressEvent& event);
		bool onMouseButtonRelease(MouseButtonReleaseEvent& event);
		bool onWindowResize(WindowResizeEvent& event);


		ParticleVertex* m_Vertices;
		ParticleInformation* m_Data;
		ParticleComponent* m_Particle;
	private:

		Ref<Scene> m_Scene;

		EditorCamera m_EditorCamera;
		AssetManager m_AssetManager;
		SceneHierarchyPanel m_SceneHierarchyPanel;

	private:	
		SceneEntity m_TestEntity;

		SpriteRenderer* m_SpriteRenderer;
		TransformComponent* m_Transform;

		glm::vec3 m_Position = { 0,0,0 };
		glm::vec3 m_Rotation = { 0,0,0 };


		bool m_MenuOpen = false;
		bool m_Selecting = false;
		bool m_LeftPanel = false;
	
		Ref<Material> m_Material;
		Ref<Texture2D> m_CharacterTexture;

		Ref<SubTexture> m_CharacterSubTexture;
		Ref<SubTexture> m_CharacterSubTexture2;
		Ref<SubTexture> m_CharacterSubTexture3;


		void* m_ProhibitedCursor = nullptr;


	private:
		struct EditorData
		{
			enum PanelID
			{
				SceneHierarchy,
				NumPanels
			};

			Entity Panel[NumPanels];
		};
	};
}