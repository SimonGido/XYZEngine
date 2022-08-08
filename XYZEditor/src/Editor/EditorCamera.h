#pragma once

#include "XYZ/Renderer/Camera.h"
#include "XYZ/Core/Timestep.h"
#include "XYZ/Event/Event.h"
#include "XYZ/Event/InputEvent.h"

#include <glm/glm.hpp>

namespace XYZ {
	namespace Editor {
		class EditorCamera : public Camera
		{
		public:
			EditorCamera();
			EditorCamera(float fov, float aspectRatio, float nearClip, float farClip);

			void OnUpdate(Timestep ts);
			void OnEvent(Event& e);

			inline float GetDistance() const { return m_Distance; }
			inline void  SetDistance(float distance) { m_Distance = distance; }
			inline void  SetViewportSize(float width, float height) { m_ViewportWidth = width; m_ViewportHeight = height; updateProjection(); }
			inline void  LockOrtho(bool val) { m_LockOrtho = val; }
			inline float GetAspectRatio() const { return m_AspectRatio; }

			const glm::mat4& GetViewMatrix() const { return m_ViewMatrix; }
			glm::mat4 GetViewProjection() const { return m_ProjectionMatrix * m_ViewMatrix; }

			glm::vec3 GetUpDirection() const;
			glm::vec3 GetRightDirection() const;
			glm::vec3 GetForwardDirection() const;
			glm::quat GetOrientation() const;
			const glm::vec3& GetPosition() const { return m_Position; }

			float GetPitch() const { return m_Pitch; }
			float GetYaw() const { return m_Yaw; }

		private:
			void updateProjection();
			void updateView();

			bool onMouseScroll(MouseScrollEvent& e);

			void mousePan(const glm::vec2& delta);
			void mouseRotate(const glm::vec2& delta);
			void mouseZoom(float delta);

			glm::vec3 calculatePosition() const;

			std::pair<float, float> panSpeed() const;
			float rotationSpeed() const;
			float zoomSpeed() const;
		private:
			float m_FOV = 45.0f, m_AspectRatio = 1.778f, m_NearClip = 0.1f, m_FarClip = -1000.0f;

			glm::mat4 m_ViewMatrix;
			glm::vec3 m_Position = { 0.0f, 0.0f, 0.0f };
			glm::vec3 m_FocalPoint = { 0.0f, 0.0f, 0.0f };

			glm::vec2 m_InitialMousePosition = { 0.0f, 0.0f };

			float m_Distance = 10.0f;
			float m_Pitch = 0.0f, m_Yaw = 0.0f;

			float m_ViewportWidth = 1280, m_ViewportHeight = 720;

			bool m_LockOrtho = false;

		};
	}
}