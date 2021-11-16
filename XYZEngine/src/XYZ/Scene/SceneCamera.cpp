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

	void SceneCamera::SetProjectionType(CameraProjectionType type)
	{
		m_ProjectionType = type;
		recalculate();
	}

	void SceneCamera::SetPerspective(const CameraPerspectiveProperties& props)
	{
		m_PerspectiveProps = props;	
	}

	void SceneCamera::SetOrthographic(const CameraOrthographicProperties& props)
	{
		m_OrthographicProps = props;
	}

	void SceneCamera::SetViewportSize(uint32_t width, uint32_t height)
	{
		m_ViewportWidth = width;
		m_ViewportHeight = height;
		recalculate();
	}
	void SceneCamera::recalculate()
	{
		switch (m_ProjectionType)
		{
		case CameraProjectionType::Perspective:
			m_ProjectionMatrix = glm::perspectiveFov(m_PerspectiveProps.PerspectiveFOV, (float)m_ViewportWidth, (float)m_ViewportHeight, m_PerspectiveProps.PerspectiveNear, m_PerspectiveProps.PerspectiveFar);
			break;
		case CameraProjectionType::Orthographic:
			const float aspect = (float)m_ViewportWidth / (float)m_ViewportHeight;
			const float w = m_OrthographicProps.OrthographicSize * aspect;
			const float h = m_OrthographicProps.OrthographicSize;
			m_ProjectionMatrix = glm::ortho(-w * 0.5f, w * 0.5f, -h * 0.5f, h * 0.5f);
			break;
		}
	}
}
