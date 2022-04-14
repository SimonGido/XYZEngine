#pragma once

#include <XYZ.h>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/matrix_decompose.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "EditorCamera.h"
#include "MarchingCubes.h"

namespace XYZ {
	class GameLayer : public Layer
	{
	public:
		GameLayer();
		virtual ~GameLayer() override;

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
		Ref<Mesh> marchingMesh(size_t& count);

	private:
		Ref<Scene>			m_Scene;
		Ref<SceneRenderer>	m_SceneRenderer;
		
		EditorCamera		m_Camera;
	
		SceneEntity			m_TestQuadEntity;

		glm::vec3 m_Min = glm::vec3(-50);
		glm::vec3 m_Max = glm::vec3( 50);
		uint32_t m_NumCellsX = 10, m_NumCellsY = 10, m_NumCellsZ = 10;
		float m_IsoLevel = 0.2f;
	};
}