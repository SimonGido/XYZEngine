#include "stdafx.h"
#include "SceneCamera.h"
#include "stdafx.h"
#include "SceneCamera.h"

#include <glm/gtc/matrix_transform.hpp>

namespace XYZ {
	SceneCamera::SceneCamera()
	{
	}

	SceneCamera::~SceneCamera()
	{
	}

	void SceneCamera::SetPerspective(float verticalFov, float nearClip, float farClip)
	{
		m_ProjectionType = CameraProjectionType::Perspective;
		m_PerspectiveFOV = verticalFov;
		m_PerspectiveNear = nearClip;
		m_PerspectiveFar = farClip;
	}
	void SceneCamera::SetOrthographic(float size, float nearClip, float farClip)
	{
		m_ProjectionType = CameraProjectionType::Orthographic;
		m_OrthographicSize = size;
		m_OrthographicNear = nearClip;
		m_OrthographicFar = farClip;
	}
	void SceneCamera::SetViewportSize(uint32_t width, uint32_t height)
	{
		switch (m_ProjectionType)
		{
		case CameraProjectionType::Perspective:
			m_ProjectionMatrix = glm::perspectiveFov(m_PerspectiveFOV, (float)width, (float)height, m_PerspectiveNear, m_PerspectiveFar);
			break;
		case CameraProjectionType::Orthographic:
			float aspect = (float)width / (float)height;
			float w = m_OrthographicSize * aspect;
			float h = m_OrthographicSize;
			m_ProjectionMatrix = glm::ortho(-w * 0.5f, w * 0.5f, -h * 0.5f, h * 0.5f);
			break;
		}
	}
}
