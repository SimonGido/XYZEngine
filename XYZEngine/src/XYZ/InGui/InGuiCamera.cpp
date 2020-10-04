#include "stdafx.h"
#include "InGuiCamera.h"
#include "XYZ/Core/Input.h"

#include <glm/gtc/matrix_transform.hpp>

namespace XYZ {
	InGuiCamera::InGuiCamera(const glm::mat4 projectionMatrix)
		: Camera(projectionMatrix)
	{
		recalculate();
	}
	void InGuiCamera::OnUpdate(float dt)
	{
		bool modified = false;
		if (Input::IsKeyPressed(KeyCode::XYZ_KEY_LEFT))
		{
			m_CameraPosition.x -= cos(glm::radians(m_CameraRotation)) * m_CameraTranslationSpeed * dt;
			m_CameraPosition.y -= sin(glm::radians(m_CameraRotation)) * m_CameraTranslationSpeed * dt;
			modified = true;
		}
		else if (Input::IsKeyPressed(KeyCode::XYZ_KEY_RIGHT))
		{
			m_CameraPosition.x += cos(glm::radians(m_CameraRotation)) * m_CameraTranslationSpeed * dt;
			m_CameraPosition.y += sin(glm::radians(m_CameraRotation)) * m_CameraTranslationSpeed * dt;
			modified = true;
		}

		if (Input::IsKeyPressed(KeyCode::XYZ_KEY_UP))
		{
			m_CameraPosition.x += -sin(glm::radians(m_CameraRotation)) * m_CameraTranslationSpeed * dt;
			m_CameraPosition.y += cos(glm::radians(m_CameraRotation)) * m_CameraTranslationSpeed * dt;
			modified = true;
		}
		else if (Input::IsKeyPressed(KeyCode::XYZ_KEY_DOWN))
		{
			m_CameraPosition.x -= -sin(glm::radians(m_CameraRotation)) * m_CameraTranslationSpeed * dt;
			m_CameraPosition.y -= cos(glm::radians(m_CameraRotation)) * m_CameraTranslationSpeed * dt;
			modified = true;
		}

		if (m_MouseMoving)
		{
			auto [mx, my] = Input::GetMousePosition();

			m_CameraPosition.x = m_OldPosition.x - ((mx - m_StartMousePos.x) * m_CameraMouseMoveSpeed * m_CameraTranslationSpeed);
			m_CameraPosition.y = m_OldPosition.y + ((my - m_StartMousePos.y) * m_CameraMouseMoveSpeed * m_CameraTranslationSpeed);

			modified = true;
		}



		if (modified)
			recalculate();
	}

	void InGuiCamera::OnEvent(Event& event)
	{
		EventDispatcher dispatcher(event);
		dispatcher.Dispatch<MouseScrollEvent>(Hook(&InGuiCamera::onMouseScrolled, this));
		dispatcher.Dispatch<MouseButtonPressEvent>(Hook(&InGuiCamera::onMouseButtonPress, this));
		dispatcher.Dispatch<MouseButtonReleaseEvent>(Hook(&InGuiCamera::onMouseButtonRelease, this));
	}

	
	void InGuiCamera::recalculate()
	{
		glm::mat4 transform = glm::translate(glm::mat4(1.0f), m_CameraPosition) *
			glm::rotate(glm::mat4(1.0f), glm::radians(m_CameraRotation), glm::vec3(0, 0, 1));

		m_ViewMatrix = glm::inverse(transform);
		m_ViewProjectionMatrix = m_ProjectionMatrix * m_ViewMatrix;
	}

	bool InGuiCamera::onMouseScrolled(MouseScrollEvent& event)
	{
		//m_ZoomLevel -= (float)event.GetOffsetY() * 0.25f;
		//m_ZoomLevel = std::max(m_ZoomLevel, 0.25f);
		//m_CameraTranslationSpeed = m_ZoomLevel;
		//
		//
		//m_ProjectionMatrix = glm::ortho(-m_AspectRatio * m_ZoomLevel, m_AspectRatio * m_ZoomLevel, m_AspectRatio * -m_ZoomLevel, m_AspectRatio * m_ZoomLevel);
		//m_ViewProjectionMatrix = m_ProjectionMatrix * m_ViewMatrix;

		return false;
	}

	bool InGuiCamera::onMouseButtonPress(MouseButtonPressEvent& event)
	{
		if (event.IsButtonPressed(MouseCode::XYZ_MOUSE_BUTTON_MIDDLE))
		{
			m_MouseMoving = true;
			auto [mx, my] = Input::GetMousePosition();
			m_StartMousePos = { mx, my };
			m_OldPosition = m_CameraPosition;
		}
		return false;
	}

	bool InGuiCamera::onMouseButtonRelease(MouseButtonReleaseEvent& event)
	{
		if (event.IsButtonReleased(MouseCode::XYZ_MOUSE_BUTTON_MIDDLE))
		{
			m_MouseMoving = false;
		}
		return false;
	}

}