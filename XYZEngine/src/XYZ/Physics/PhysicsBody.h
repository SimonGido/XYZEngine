#pragma once
#include "PhysicsShape.h"

#include <glm/glm.hpp>

namespace XYZ {

	struct Fixture
	{
		PhysicsShape* Shape;
		float Density;
	};

	class PhysicsBody
	{
	public:
		PhysicsBody(const glm::vec2& position, float angle, uint32_t id);
		
		void SetFixtureDensity(uint32_t index, float density) { m_Fixtures[index].Density = density; recalculateMass(); }

		const glm::vec2& GetPosition() const { return m_Position; }
		float GetMass() const { return m_Mass;  }
		float GetAngle() const { return m_Angle; }
		const std::vector<Fixture>& GetFixtures() const { return m_Fixtures; }



		enum class Type { Dynamic, Kinematic, Static };


		Type m_Type = Type::Dynamic;
		float m_LinearDamping = 0.0f;
		float m_AngularDamping = 0.0f;
		glm::vec2 m_LinearVelocity = glm::vec2(0.0f);

	private:
		void recalculateMass();
		
	private:
		glm::vec2 m_Position;
		float	  m_Angle;

		float     m_AngularVelocity = 0.0f;
		float	  m_Mass = 0.0f;


		const uint32_t m_ID;
		
		std::vector<Fixture> m_Fixtures;
		friend class PhysicsWorld;
	};
}