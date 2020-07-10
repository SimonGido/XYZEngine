#pragma once
#include "XYZ/Renderer/Camera.h"


namespace XYZ {


	class SceneCamera : public Camera
	{
	public:
		SceneCamera();
		virtual ~SceneCamera();

		void SetPerspective(float verticalFov, float nearClip = 0.01f, float farClip = 10000.0f);
		void SetOrthographic(float size, float nearClip = -1.0f, float farClip = 1.0f);

		void SetViewportSize(uint32_t width, uint32_t height);

		CameraProjectionType GetProjectionType() const { return m_ProjectionType; }
	private:
		CameraProjectionType m_ProjectionType = CameraProjectionType::Orthographic;

		float m_PerspectiveFOV = 1.0f;
		float m_PerspectiveNear = 0.01f;
		float m_PerspectiveFar = 10000.0f;

		float m_OrthographicSize = 10.0f;
		float m_OrthographicNear = -1.0f;
		float m_OrthographicFar = 1.0f;
	};
}