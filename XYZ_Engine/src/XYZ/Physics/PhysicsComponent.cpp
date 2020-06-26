#include "stdafx.h"
#include "PhysicsComponent.h"




namespace XYZ {
	Transform2D::Transform2D(const glm::vec3& pos, const glm::vec2& scale, float rot)
		:
		m_Position(pos),
		m_Scale(scale),
		m_Rotation(rot)
	{
		m_Transformation = calculateTransform();
	}

	void Transform2D::Translate(const glm::vec3& translation)
	{
		m_Position += translation;
		m_Updated = true;
	}
	void Transform2D::Scale(const glm::vec2& scale)
	{
		m_Scale += scale;
		m_Updated = true;
	}
	void Transform2D::Rotate(float rotation)
	{
		m_Rotation += rotation;
		m_Updated = true;
	}
	void Transform2D::InheritParent(const Transform2D& parent)
	{
		if (m_Parent && m_Parent->m_Updated)
			m_Transformation = m_Parent->m_Transformation * calculateTransform();
	}

	void Transform2D::CalculateWorldTransformation()
	{
		// If has parent
		if (m_Parent)
		{
			// If parent was updated or child was updated recalculate
			if (m_Parent->m_Updated || m_Updated)
			{
				m_Transformation = m_Parent->m_Transformation * calculateTransform();
				m_Updated = true;
			}
		}
		// If does not have parent
		else if (m_Updated)
		{
			m_Transformation = calculateTransform();
			m_Updated = true;
		}	
	}

	void Transform2D::DetachParent()
	{
		if (m_Parent)
		{
			m_Parent = nullptr;
			m_Transformation = calculateTransform();
		}
		else
			XYZ_LOG_WARN("Transform has no parent to detach");
	}

	
	const glm::mat4& Transform2D::GetTransformation() const
	{
		m_Updated = false;
		return m_Transformation;
	}

	
	glm::mat4 Transform2D::calculateTransform() const
	{
		glm::mat4 posMatrix = glm::translate(m_Position);
		glm::mat4 rotMatrix = glm::rotate(m_Rotation, glm::vec3(0, 0, 1));
		glm::mat4 scaleMatrix = glm::scale(glm::vec3(m_Scale, 1));
		return posMatrix * rotMatrix * scaleMatrix;
	}

}