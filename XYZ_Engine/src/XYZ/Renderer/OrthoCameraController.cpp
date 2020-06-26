#include "stdafx.h"
#include "OrthoCameraController.h"

#include "XYZ/Core/KeyCodes.h"
#include "XYZ/Core/Input.h"

namespace XYZ {
	OrthoCameraController::OrthoCameraController(float aspectRatio, bool rotation)
		:
		m_AspectRatio(aspectRatio),
		m_Bounds({ -m_AspectRatio * m_ZoomLevel, m_AspectRatio * m_ZoomLevel, -m_ZoomLevel, m_ZoomLevel }),
		m_Camera(-m_AspectRatio * m_ZoomLevel, m_AspectRatio* m_ZoomLevel, -m_ZoomLevel, m_ZoomLevel), m_Rotation(rotation)
	{
		m_MouseScroll = EventManager::Get().AddHandler(EventType::MouseScroll, std::bind(&OrthoCameraController::OnMouseScrolled, this, std::placeholders::_1));
		m_WindowResize = EventManager::Get().AddHandler(EventType::WindowResized, std::bind(&OrthoCameraController::OnWindowResized, this, std::placeholders::_1));
	}

	OrthoCameraController::~OrthoCameraController()
	{
		EventManager::Get().RemoveHandler(EventType::MouseScroll, m_MouseScroll);
		EventManager::Get().RemoveHandler(EventType::WindowResized, m_WindowResize);
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

	void OrthoCameraController::OnMouseScrolled(event_ptr event)
	{
		std::shared_ptr<MouseScrollEvent> e = std::static_pointer_cast<MouseScrollEvent>(event);
		m_ZoomLevel -= (float)e->GetOffsetY() * 0.25f;
		m_ZoomLevel = std::max(m_ZoomLevel, 0.25f);
		m_Camera.SetProjection(-m_AspectRatio * m_ZoomLevel, m_AspectRatio * m_ZoomLevel, -m_ZoomLevel, m_ZoomLevel);
	}

	void OrthoCameraController::OnWindowResized(event_ptr event)
	{
		std::shared_ptr<WindowResizeEvent> e = std::static_pointer_cast<WindowResizeEvent>(event);
		m_AspectRatio = (float)e->GetWidth() / (float)e->GetHeight();
		m_Camera.SetProjection(-m_AspectRatio * m_ZoomLevel, m_AspectRatio * m_ZoomLevel, -m_ZoomLevel, m_ZoomLevel);
	}
}