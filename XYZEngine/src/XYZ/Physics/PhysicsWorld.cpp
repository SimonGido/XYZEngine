#include "stdafx.h"
#include "PhysicsWorld.h"

#include "XYZ/Scene/Components.h"

namespace XYZ {


	PhysicsWorld::PhysicsWorld(const glm::vec2& gravity)
		:
		m_Pool(10 * 1024),
		m_Gravity(gravity)
	{
		m_Tree.Insert(NULL_ENTITY, AABB(glm::vec3(-200.0f), glm::vec3(200.0f)));
	}

	void PhysicsWorld::Update(Timestep ts, float updateFrequency)
	{
		if (m_CurrentTime >= updateFrequency)
		{
			m_CurrentTime = 0.0f;
			for (auto body : m_Bodies)
			{
				if (body->m_Type != PhysicsBody::Type::Static)
				{
					glm::vec2 old = body->m_Position;
					body->m_Position += m_Gravity * body->m_Mass * updateFrequency;
					for (auto& fixture : body->m_Fixtures)
					{
						float inertia = fixture.Shape->CalculateInertia(body->m_Mass);
						float torque = fixture.Shape->CalculateTorque(glm::vec2(5.0f, m_Gravity.y));
						body->m_Angle += torque / inertia;
						m_Tree.Move(fixture.Shape->m_ID, body->m_Position - old);
					}
				}
			}
		}
		m_CurrentTime += ts;
		m_Tree.SubmitToRenderer();
	}

	PhysicsBody* PhysicsWorld::CreateBody(const glm::vec2& position, float rotation)
	{
		PhysicsBody* body = m_Pool.Allocate<PhysicsBody>(position, rotation, m_Bodies.size());
		m_Bodies.push_back(body);
		return body;
	}

	BoxShape2D* PhysicsWorld::AddBox2DShape(PhysicsBody* body, const glm::vec2& min, const glm::vec2& max, float density)
	{
		BoxShape2D* box = m_Pool.Allocate<BoxShape2D>(min, max);
		body->m_Fixtures.push_back({ box, density });
		body->recalculateMass();
		box->m_ID = m_Tree.Insert(body->m_ID, box->GetAABB());
		return box;
	}

	CircleShape* PhysicsWorld::AddCircleShape(PhysicsBody* body, float radius, float density)
	{
		CircleShape* circle = m_Pool.Allocate<CircleShape>(radius);
		body->m_Fixtures.push_back({ circle, density });
		body->recalculateMass();
		circle->m_ID = m_Tree.Insert(body->m_ID, circle->GetAABB());
		return circle;
	}
	
}