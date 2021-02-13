#pragma once

#include <XYZ.h>

#include "Panels/SceneHierarchyPanel.h"
#include "Panels/ScenePanel.h"
#include "Panels/InspectorPanel.h"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/matrix_decompose.hpp>
#include <glm/gtc/type_ptr.hpp>



namespace XYZ {
	namespace PanelID {
		enum PanelID
		{
			SceneHierarchyPanel,
			ScenePanel,
			InspectorPanel,
			NumPanels
		};
	}
	
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
		bool onKeyPress(KeyPressedEvent& event);
		bool onKeyRelease(KeyReleasedEvent& event);

		ParticleVertex* m_Vertices;
		ParticleInformation* m_Data;
		ParticleComponent* m_Particle;
	private:
		Ref<Scene> m_Scene;

		AssetManager m_AssetManager;
		SceneHierarchyPanel m_SceneHierarchyPanel;
		InspectorPanel m_InspectorPanel;
		ScenePanel m_ScenePanel;


	private:	
		SceneEntity m_TestEntity;
		SceneEntity m_NewEntity;
		SceneEntity m_SelectedEntity;

		SpriteRenderer* m_SpriteRenderer;
		TransformComponent* m_Transform;

		glm::vec3 m_Position = { 0,0,0 };
		glm::vec3 m_Rotation = { 0,0,0 };
	
		Ref<Material> m_Material;
		Ref<Texture2D> m_CharacterTexture;

		Ref<SubTexture> m_CharacterSubTexture;
		Ref<SubTexture> m_CharacterSubTexture2;
		Ref<SubTexture> m_CharacterSubTexture3;
	};
}