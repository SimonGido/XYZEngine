#pragma once
#include "PhysicsShape.h"

#include <glm/glm.hpp>

namespace XYZ {

	class PhysicsBody
	{
	public:
		PhysicsBody(const glm::vec2& position, float angle, uint32_t id);
			

		const glm::vec2& GetPosition() const { return m_Position; }
		const float GetAngle() const { return m_Angle; }

	private:
		glm::vec2 m_Position;
		float	  m_Angle;
		
		std::vector<PhysicsShape*> m_ShapeAttachments;

		const uint32_t m_ID;

		friend class PhysicsWorld;
	};
}