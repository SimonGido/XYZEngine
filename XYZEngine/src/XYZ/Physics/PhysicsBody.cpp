#include "stdafx.h"
#include "PhysicsBody.h"

#include "XYZ/Utils/Math/Math.h"

namespace XYZ {
	PhysicsBody::PhysicsBody(const glm::vec2& position, float angle, uint32_t id)
		:
		m_Position(position),
		m_OldPosition(position),
		m_Angle(angle),
		m_ID(id)
	{}
	void PhysicsBody::ApplyImpulse(const glm::vec2 & impulse, const glm::vec2 & contactVector)
	{
		m_Velocity += m_InverseInertia * impulse;
		m_AngularVelocity += m_InverseInertia * Math::Cross(contactVector, impulse);
	}
	void PhysicsBody::AddVelocity(const glm::vec2& velocity)
	{
		m_Velocity += velocity;
	}
	void PhysicsBody::SetType(Type type)
	{
		m_Type = type;
		if (m_Type == Type::Static)
		{
			m_InverseInertia = 0.0f;
			m_InverseMass = 0.0f;
		}
	}
	void PhysicsBody::recalculateMass()
	{
		m_Mass = m_Shape->CalculateMass(m_Density);
		m_InverseMass = 1.0f / m_Mass;
		if (m_Type == Type::Static)
		{
			m_Mass = 0.0f;
			m_InverseMass = 0.0f;
		}
	}
}