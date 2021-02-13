#include "stdafx.h"
#include "PhysicsBody.h"


namespace XYZ {
	PhysicsBody::PhysicsBody(const glm::vec2& position, float angle, uint32_t id)
		:
		m_Position(position),
		m_OldPosition(position),
		m_Angle(angle),
		m_ID(id)
	{}
	void PhysicsBody::recalculateMass()
	{
		m_Mass = 0.0f;
		for (auto& fixture : m_Fixtures)
		{
			m_Mass += fixture.Shape->CalculateMass(fixture.Density);
		}
	}
}