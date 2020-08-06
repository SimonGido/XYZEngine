#include "stdafx.h"
#include "Transform.h"


#include <glm/gtx/transform.hpp>

namespace XYZ {
	Transform::Transform(const glm::vec3& pos, const glm::vec2& scale, float rot)
		:
		m_Position(pos),
		m_Scale(scale),
		m_Rotation(rot)
	{
		m_Transformation = calculateTransform();
	}

	void Transform::Translate(const glm::vec3& translation)
	{
		m_Position += translation;
		m_Updated = true;
	}
	void Transform::Scale(const glm::vec2& scale)
	{
		m_Scale += scale;
		m_Updated = true;
	}
	void Transform::Rotate(float rotation)
	{
		m_Rotation += rotation;
		m_Updated = true;
	}
	void Transform::SetTranslation(const glm::vec3& translation)
	{
		m_Position = translation;
		m_Updated = true;
	}
	void Transform::SetScale(const glm::vec2& scale)
	{
		m_Scale = scale;
		m_Updated = true;
	}
	void Transform::SetRotation(float rotation)
	{
		m_Rotation = rotation;
		m_Updated = true;
	}

	void Transform::SetParent(Transform* parent)
	{
		m_Parent = parent;
		m_Transformation = m_Parent->m_Transformation * calculateTransform();
	}

	void Transform::CalculateWorldTransformation()
	{
		// If has parent
		if (m_Parent)
		{
			// If parent was updated or child was updated recalculate
			if (m_Parent->m_Updated || m_Updated)
			{
				m_Transformation = m_Parent->m_Transformation * calculateTransform();
			}
		}
		// If does not have parent
		else if (m_Updated)
		{
			m_Transformation = calculateTransform();
		}
	}

	void Transform::DetachParent()
	{
		if (m_Parent)
		{
			m_Parent = nullptr;
			m_Transformation = calculateTransform();
		}
		else
			XYZ_LOG_WARN("Transform has no parent to detach");
	}


	const glm::vec3 Transform::GetWorldPosition() const
	{
		return {
			m_Transformation[3][0],
			m_Transformation[3][1],
			m_Transformation[3][2]
		};
	}

	const glm::vec2 Transform::GetWorldScale() const
	{
		return {
			glm::length(m_Transformation[0]),
			glm::length(m_Transformation[1])
		};
	}


	const glm::mat4& Transform::GetTransformation() const
	{
		m_Updated = false;
		return m_Transformation;
	}


	glm::mat4 Transform::calculateTransform() const
	{
		glm::mat4 posMatrix = glm::translate(m_Position);
		glm::mat4 rotMatrix = glm::rotate(m_Rotation, glm::vec3(0, 0, 1));
		glm::mat4 scaleMatrix = glm::scale(glm::vec3(m_Scale, 1));
		return posMatrix * rotMatrix * scaleMatrix;
	}
}