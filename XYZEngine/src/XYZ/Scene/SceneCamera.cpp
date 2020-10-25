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

	void SceneCamera::SetPerspective(const SceneCameraPerspectiveProperties& perspectiveProps)
	{
		m_PerspectiveProperties = perspectiveProps;
	}
	void SceneCamera::SetOrthographic(const SceneCameraOrthographicProperties& orthoProps)
	{
		m_OrthographicProperties = orthoProps;
	}
	void SceneCamera::SetViewportSize(uint32_t width, uint32_t height)
	{
		switch (m_ProjectionType)
		{
		case CameraProjectionType::Perspective:
			m_ProjectionMatrix = glm::perspectiveFov(m_PerspectiveProperties.PerspectiveFOV, (float)width, (float)height, m_PerspectiveProperties.PerspectiveNear, m_PerspectiveProperties.PerspectiveFar);
			break;
		case CameraProjectionType::Orthographic:
			float aspect = (float)width / (float)height;
			float w = m_OrthographicProperties.OrthographicSize * aspect;
			float h = m_OrthographicProperties.OrthographicSize;
			m_ProjectionMatrix = glm::ortho(-w * 0.5f, w * 0.5f, -h * 0.5f, h * 0.5f);
			break;
		}
	}
}
