#pragma once
#include "XYZ/Renderer/Camera.h"
#include "XYZ/Event/Event.h"
#include "XYZ/Event/InputEvent.h"
#include "XYZ/Event/ApplicationEvent.h"

namespace XYZ {
	class EditorCamera : public Camera
	{
	public:
		EditorCamera() = default;
		EditorCamera(const glm::mat4 projectionMatrix);


		void OnUpdate(float dt);
		void OnEvent(Event& event);

		const glm::vec3& GetPosition() const { return m_CameraPosition; }
		float GetRotation() const { return m_CameraRotation; }

		const glm::mat4& GetViewProjectionMatrix() const { return m_ViewProjectionMatrix; }
	private:

		void recalculate();
		/**
		* Handler for the mouse scrolled event
		* @param[in] event shared_ptr to the Event
		*/
		bool onMouseScrolled(MouseScrollEvent& event);

		/**
		* Handler for the window resized event
		* @param[in] event shared_ptr to the Event
		*/
		bool onWindowResized(WindowResizeEvent& event);

	private:
		glm::mat4 m_ViewProjectionMatrix = glm::mat4(1.0f);
		glm::mat4 m_ViewMatrix = glm::mat4(1.0f);

		float m_AspectRatio = 16.0f / 9.0f;
		float m_ZoomLevel = 1.0f;

		glm::vec3 m_CameraPosition = { 0.0f, 0.0f, 0.0f };
		float m_CameraRotation = 0.0f; //In degrees, in the anti-clockwise direction
		float m_CameraTranslationSpeed = 5.0f, m_CameraRotationSpeed = 180.0f;
	};
}