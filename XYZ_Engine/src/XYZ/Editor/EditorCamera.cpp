#include "stdafx.h"
#include "EditorCamera.h"
#include "XYZ/Core/Input.h"

#include <glm/gtc/matrix_transform.hpp>

namespace XYZ {
	EditorCamera::EditorCamera(const glm::mat4 projectionMatrix)
		: Camera(projectionMatrix)
	{
		recalculate();
	}
	void EditorCamera::OnUpdate(float dt)
	{
		bool modified = false;
		if (Input::IsKeyPressed(KeyCode::XYZ_KEY_A))
		{
			m_CameraPosition.x -= cos(glm::radians(m_CameraRotation)) * m_CameraTranslationSpeed * dt;
			m_CameraPosition.y -= sin(glm::radians(m_CameraRotation)) * m_CameraTranslationSpeed * dt;
			modified = true;
		}
		else if (Input::IsKeyPressed(KeyCode::XYZ_KEY_D))
		{
			m_CameraPosition.x += cos(glm::radians(m_CameraRotation)) * m_CameraTranslationSpeed * dt;
			m_CameraPosition.y += sin(glm::radians(m_CameraRotation)) * m_CameraTranslationSpeed * dt;
			modified = true;
		}

		if (Input::IsKeyPressed(KeyCode::XYZ_KEY_W))
		{
			m_CameraPosition.x += -sin(glm::radians(m_CameraRotation)) * m_CameraTranslationSpeed * dt;
			m_CameraPosition.y += cos(glm::radians(m_CameraRotation)) * m_CameraTranslationSpeed * dt;
			modified = true;
		}
		else if (Input::IsKeyPressed(KeyCode::XYZ_KEY_S))
		{
			m_CameraPosition.x -= -sin(glm::radians(m_CameraRotation)) * m_CameraTranslationSpeed * dt;
			m_CameraPosition.y -= cos(glm::radians(m_CameraRotation)) * m_CameraTranslationSpeed * dt;
			modified = true;
		}

		
		if (Input::IsKeyPressed(KeyCode::XYZ_KEY_Q))
		{
			m_CameraRotation += m_CameraRotationSpeed * dt;
			modified = true;
		}
		if (Input::IsKeyPressed(KeyCode::XYZ_KEY_E))
		{
			m_CameraRotation -= m_CameraRotationSpeed * dt;
			modified = true;
		}
		if (m_CameraRotation > 180.0f)
		{
			m_CameraRotation -= 360.0f;
			modified = true;
		}
		else if (m_CameraRotation <= -180.0f)
		{
			m_CameraRotation += 360.0f;
			modified = true;
		}

		m_CameraTranslationSpeed = m_ZoomLevel;

		if (modified)
			recalculate();
	}
	void EditorCamera::OnEvent(Event& event)
	{
		EventDispatcher dispatcher(event);
		dispatcher.Dispatch<MouseScrollEvent>(Hook(&EditorCamera::onMouseScrolled, this));
		dispatcher.Dispatch<WindowResizeEvent>(Hook(&EditorCamera::onWindowResized, this));
	}
	void EditorCamera::recalculate()
	{
		glm::mat4 transform = glm::translate(glm::mat4(1.0f), m_CameraPosition) *
			glm::rotate(glm::mat4(1.0f), glm::radians(m_CameraRotation), glm::vec3(0, 0, 1));

		m_ViewMatrix = glm::inverse(transform);
		m_ViewProjectionMatrix = m_ProjectionMatrix * m_ViewMatrix;
	}
	bool EditorCamera::onMouseScrolled(MouseScrollEvent& event)
	{
		m_ZoomLevel -= (float)event.GetOffsetY() * 0.25f;
		m_ZoomLevel = std::max(m_ZoomLevel, 0.25f);

		m_ProjectionMatrix = glm::ortho(-m_AspectRatio * m_ZoomLevel, m_AspectRatio * m_ZoomLevel, m_AspectRatio * -m_ZoomLevel, m_AspectRatio * m_ZoomLevel);
		m_ViewProjectionMatrix = m_ProjectionMatrix * m_ViewMatrix;

		return false;
	}
	bool EditorCamera::onWindowResized(WindowResizeEvent& event)
	{
		m_AspectRatio = (float)event.GetWidth() / (float)event.GetHeight();
		m_ProjectionMatrix = glm::ortho(-m_AspectRatio * m_ZoomLevel, m_AspectRatio * m_ZoomLevel, m_AspectRatio * -m_ZoomLevel, m_AspectRatio * m_ZoomLevel);
		m_ViewProjectionMatrix = m_ProjectionMatrix * m_ViewMatrix;
		return false;
	}
}