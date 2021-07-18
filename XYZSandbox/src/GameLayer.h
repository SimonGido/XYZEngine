#pragma once

#include <XYZ.h>

#include "Util/OrthographicCameraController.h"
#include "Automata/Grid.h"

namespace XYZ {

	class GameLayer : public Layer
	{
	public:
		GameLayer();
		virtual ~GameLayer();

		virtual void OnAttach() override;
		virtual void OnDetach() override;
		virtual void OnUpdate(Timestep ts) override;
		virtual void OnEvent(Event& event) override;
		virtual void OnImGuiRender() override;

	private:
		bool onWindowResize(WindowResizeEvent& event);

	private:
		OrthographicCameraController m_CameraController;

		Renderer2DStats m_RendererStats;
		float			m_Timestep;
		glm::vec2		m_MousePosition;
		Project::Grid	m_Grid;
	};

}