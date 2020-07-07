#include "stdafx.h"
#include "OrthoCameraController.h"

#include "XYZ/Core/KeyCodes.h"
#include "XYZ/Core/Input.h"

namespace XYZ {
	OrthoCameraController::OrthoCameraController(float aspectRatio, bool rotation)
		:
		m_AspectRatio(aspectRatio),
		m_Camera(-m_AspectRatio * m_ZoomLevel, m_AspectRatio* m_ZoomLevel, -m_ZoomLevel, m_ZoomLevel), 
		m_Rotation(rotation)
	{
	}

	OrthoCameraController::~OrthoCameraController()
	{
		
	}

	void OrthoCameraController::OnUpdate(float dt)
	{
		if (Input::IsKeyPressed(KeyCode::XYZ_KEY_A))
		{
			m_CameraPosition.x -= cos(glm::radians(m_CameraRotation)) * m_CameraTranslationSpeed * dt;
			m_CameraPosition.y -= sin(glm::radians(m_CameraRotation)) * m_CameraTranslationSpeed * dt;
		}
		else if (Input::IsKeyPressed(KeyCode::XYZ_KEY_D))
		{
			m_CameraPosition.x += cos(glm::radians(m_CameraRotation)) * m_CameraTranslationSpeed * dt;
			m_CameraPosition.y += sin(glm::radians(m_CameraRotation)) * m_CameraTranslationSpeed * dt;
		}

		if (Input::IsKeyPressed(KeyCode::XYZ_KEY_W))
		{
			m_CameraPosition.x += -sin(glm::radians(m_CameraRotation)) * m_CameraTranslationSpeed * dt;
			m_CameraPosition.y += cos(glm::radians(m_CameraRotation)) * m_CameraTranslationSpeed * dt;
		}
		else if (Input::IsKeyPressed(KeyCode::XYZ_KEY_S))
		{
			m_CameraPosition.x -= -sin(glm::radians(m_CameraRotation)) * m_CameraTranslationSpeed * dt;
			m_CameraPosition.y -= cos(glm::radians(m_CameraRotation)) * m_CameraTranslationSpeed * dt;
		}

		if (m_Rotation)
		{
			if (Input::IsKeyPressed(KeyCode::XYZ_KEY_Q))
				m_CameraRotation += m_CameraRotationSpeed * dt;
			if (Input::IsKeyPressed(KeyCode::XYZ_KEY_E))
				m_CameraRotation -= m_CameraRotationSpeed * dt;

			if (m_CameraRotation > 180.0f)
				m_CameraRotation -= 360.0f;
			else if (m_CameraRotation <= -180.0f)
				m_CameraRotation += 360.0f;

			m_Camera.SetRotation(m_CameraRotation);
		}

		m_Camera.SetPosition(m_CameraPosition);

		m_CameraTranslationSpeed = m_ZoomLevel;
	}

	void OrthoCameraController::OnEvent(Event& event)
	{
		if (event.GetEventType() == EventType::MouseScroll)
		{
			MouseScrollEvent& e = (MouseScrollEvent&)event;
			OnMouseScrolled(e);
		}
		else if (event.GetEventType() == EventType::WindowResized)
		{
			WindowResizeEvent& e = (WindowResizeEvent&)event;
			OnWindowResized(e);
		}
	} 

	void OrthoCameraController::OnMouseScrolled(MouseScrollEvent& event)
	{	
		m_ZoomLevel -= (float)event.GetOffsetY() * 0.25f;
		m_ZoomLevel = std::max(m_ZoomLevel, 0.25f);
		m_Camera.SetProjection(-m_AspectRatio * m_ZoomLevel, m_AspectRatio * m_ZoomLevel, -m_ZoomLevel, m_ZoomLevel);
	}

	void OrthoCameraController::OnWindowResized(WindowResizeEvent& event)
	{
		m_AspectRatio = (float)event.GetWidth() / (float)event.GetHeight();
		m_Camera.SetProjection(-m_AspectRatio * m_ZoomLevel, m_AspectRatio * m_ZoomLevel, -m_ZoomLevel, m_ZoomLevel);
	}
}