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
		enum class Type { Dynamic, Kinematic, Static };


		PhysicsBody(const glm::vec2& position, float angle, uint32_t id);
		
		void SetDensity(float density) { m_Density = density; recalculateMass(); }
		void ApplyImpulse(const glm::vec2& impulse, const glm::vec2& contactVector);
		void AddVelocity(const glm::vec2& velocity);
		void SetType(Type type);

		const glm::vec2& GetPosition() const { return m_Position; }
		const glm::vec2& GetVelocity() const { return m_Velocity; }
		
		float GetMass() const { return m_Mass;  }
		float GetAngle() const { return m_Angle; }
		uint32_t GetID() const { return m_ID; }
		const PhysicsShape* GetShape() const { return m_Shape; }

	private:
		void recalculateMass();
		
	private:
		Type m_Type = Type::Dynamic;
		glm::vec2 m_OldPosition;
		glm::vec2 m_Position;
		float	  m_Angle;

		glm::vec2 m_Forces = glm::vec2(0.0f);
		glm::vec2 m_Velocity = glm::vec2(0.0f);
		float     m_AngularVelocity = 0.0f;
		float	  m_Mass = 0.0f;
		float	  m_InverseMass = 0.0f;
		float	  m_InverseInertia = 0.0f;
		float     m_StaticFriction = 0.5f;
		float     m_DynamicFriction = 0.5f;
		float	  m_Restitution = 0.5f;
		float     m_Torque = 0.0f;
		float	  m_Density = 1.0f;
		PhysicsShape* m_Shape = nullptr;
		

		const uint32_t m_ID;

		friend class Manifold;
		friend class PhysicsWorld;
	};
}