#include "stdafx.h"
#include "Camera.h"
#include "stdafx.h"
#include "Camera.h"



namespace XYZ {
	Camera::Camera(const glm::mat4& projectionMatrix)
		: m_ProjectionMatrix(projectionMatrix)
	{
	}
	void Camera::SetPerspective(const CameraPerspectiveProperties& perspectiveProps)
	{
		m_PerspectiveProperties = perspectiveProps;
	}
	void Camera::SetOrthographic(const CameraOrthographicProperties& orthoProps)
	{
		m_OrthographicProperties = orthoProps;
	}
}
