#include "stdafx.h"
#include "PhysicsBody.h"


namespace XYZ {
	PhysicsBody::PhysicsBody(const glm::vec2& position, float angle, uint32_t id)
		:
		m_Position(position),
		m_Angle(angle),
		m_ID(id)
	{}
}