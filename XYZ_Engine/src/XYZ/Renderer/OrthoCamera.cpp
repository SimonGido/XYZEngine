#include "stdafx.h"
#include "OrthoCamera.h"

#include "glm/gtc/matrix_transform.hpp"


namespace XYZ {
	void OrthoCamera::recalculateViewMatrix()
	{
		glm::mat4 transform = glm::translate(glm::mat4(1.0f), m_Position) *
			glm::rotate(glm::mat4(1.0f), glm::radians(m_Rotation), glm::vec3(0, 0, 1));

		m_ViewMatrix = glm::inverse(transform);
		m_ViewProjectionMatrix = m_ProjectionMatrix * m_ViewMatrix;
	}
	OrthoCamera::OrthoCamera(float left, float right, float bottom, float top)
		: m_ProjectionMatrix(glm::ortho(left, right, bottom, top, -1.0f, 1.0f)), m_ViewMatrix(1.0f), m_Position(0.0f)
	{
		m_ViewProjectionMatrix = m_ProjectionMatrix * m_ViewMatrix;
	}
	void OrthoCamera::Bind()
	{
		if (m_RenderTarget.get())
			m_RenderTarget->Bind();
	}
	void OrthoCamera::UnBind()
	{
		if (m_RenderTarget.get())
			m_RenderTarget->Unbind();
	}
	void OrthoCamera::SetProjection(float left, float right, float bottom, float top)
	{
		m_ProjectionMatrix = glm::ortho(left, right, bottom, top);
		m_ViewProjectionMatrix = m_ProjectionMatrix * m_ViewMatrix;
	}

}