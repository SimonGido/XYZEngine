#pragma once
#include "XYZ/Utils/DataStructures/MemoryPool.h"
#include "XYZ/Core/Timestep.h"
#include "DynamicTree.h"
#include "PhysicsBody.h"
#include "PhysicsShape.h"
#include "Manifold.h"

#include <glm/glm.hpp>

namespace XYZ {

	class PhysicsWorld
	{
	public:
		PhysicsWorld(const glm::vec2& gravity);

		void Update(Timestep ts);

		void DestroyBody(PhysicsBody* body);
		void DestroyShape(PhysicsShape* shape);

		PhysicsBody* CreateBody(const glm::vec2& position, float rotation);
		BoxShape2D* AddBox2DShape(PhysicsBody* body, const glm::vec2& min, const glm::vec2& max, float density);
		CircleShape* AddCircleShape(PhysicsBody* body, float radius, float density);
		PolygonShape* AddPolygonShape(PhysicsBody* body, const std::vector<glm::vec2>& vertices, float density);
	private:
		void applyForces();
		void applyVelocities();
		void broadPhase();
		void narrowPhase();

		void debugDraw();
	private:
		DynamicTree m_Tree;
		MemoryPool m_Pool;

		glm::vec2 m_Gravity;


		std::vector<PhysicsBody*> m_Bodies;
		std::vector<Manifold> m_Manifolds;

		float m_DeltaTime = 0.0f;
	};
}