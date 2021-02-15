#include "stdafx.h"
#include "PhysicsWorld.h"

#include "XYZ/Scene/Components.h"
#include "XYZ/Renderer/Renderer2D.h"

namespace XYZ {

	static void SubmitBoxToRenderer(const AABB& box, const glm::vec4& color)
	{
		Renderer2D::SubmitLine(box.Min, glm::vec3(box.Max.x, box.Min.y, box.Min.z), color);
		Renderer2D::SubmitLine(glm::vec3(box.Max.x, box.Min.y, box.Min.z), glm::vec3(box.Max.x, box.Max.y, box.Min.z), color);
		Renderer2D::SubmitLine(glm::vec3(box.Max.x, box.Max.y, box.Min.z), glm::vec3(box.Min.x, box.Max.y, box.Min.z), color);
		Renderer2D::SubmitLine(glm::vec3(box.Min.x, box.Max.y, box.Min.z), box.Min, color);


		Renderer2D::SubmitLine(glm::vec3(box.Min.x, box.Min.y, box.Max.z), glm::vec3(box.Max.x, box.Min.y, box.Max.z), color);
		Renderer2D::SubmitLine(glm::vec3(box.Max.x, box.Min.y, box.Max.z), glm::vec3(box.Max.x, box.Max.y, box.Max.z), color);
		Renderer2D::SubmitLine(glm::vec3(box.Max.x, box.Max.y, box.Max.z), glm::vec3(box.Min.x, box.Max.y, box.Max.z), color);
		Renderer2D::SubmitLine(glm::vec3(box.Min.x, box.Max.y, box.Max.z), glm::vec3(box.Min.x, box.Min.y, box.Max.z), color);


		Renderer2D::SubmitLine(box.Min, glm::vec3(box.Min.x, box.Min.y, box.Max.z), color);
		Renderer2D::SubmitLine(glm::vec3(box.Min.x, box.Max.y, box.Min.z), glm::vec3(box.Min.x, box.Max.y, box.Max.z), color);


		Renderer2D::SubmitLine(glm::vec3(box.Max.x, box.Min.y, box.Min.z), glm::vec3(box.Max.x, box.Min.y, box.Max.z), color);
		Renderer2D::SubmitLine(glm::vec3(box.Max.x, box.Max.y, box.Min.z), glm::vec3(box.Max.x, box.Max.y, box.Max.z), color);
	}

	PhysicsWorld::PhysicsWorld(const glm::vec2& gravity)
		:
		m_Pool(10 * 1024),
		m_Gravity(gravity)
	{
	}

	void PhysicsWorld::Update(Timestep ts)
	{	
		broadPhase(ts);
		// Go through possible intersections and resolve collisions
		for (auto& body : m_Bodies)
			body->m_OldPosition = body->m_Position;
		

		for (auto& it : m_IntersectingNodes)
		{
			auto firstBody = m_Bodies[m_Tree.GetObjectIndex(it.first)];
			auto secondBody = m_Bodies[m_Tree.GetObjectIndex(it.second)];


			for (auto& fixture : firstBody->GetFixtures())
			{
				for (auto& otherFixture : secondBody->GetFixtures())
				{
					{
						auto data = fixture.Shape->Intersect(*otherFixture.Shape, ts);
						if (data.Intersection)
						{
							if (firstBody->m_Type != PhysicsBody::Type::Static)
							{
								firstBody->m_Position -= data.Displacement;
								firstBody->m_Velocity = -firstBody->m_Velocity * 0.2f;
							}
						}
					}
					{
						auto data = otherFixture.Shape->Intersect(*fixture.Shape, ts);
						if (data.Intersection)
						{
							if (secondBody->m_Type != PhysicsBody::Type::Static)
							{
								secondBody->m_Position -= data.Displacement;
								secondBody->m_Velocity = -secondBody->m_Velocity * 0.2f;
							}
						}
					}
				}
			}
		}
		// Restart intersecting nodes and moved nodes
		m_IntersectingNodes.clear();
		m_Tree.CleanMovedNodes();

		// Apply velocity and forces
		for (auto& body : m_Bodies)
		{
			if (body->m_Type != PhysicsBody::Type::Static)
			{
				body->m_Velocity += (m_Gravity + (body->m_Forces / body->m_Mass)) * (float)ts;
				body->m_Position += body->m_Velocity * (float)ts;
				if (body->m_Position != body->m_OldPosition)
				{
					for (auto& fixture : body->m_Fixtures)
					{
						m_Tree.Move(fixture.Shape->m_ID, body->m_Position - body->m_OldPosition);
					}
				}
				body->m_Forces = glm::vec2(0.0f);
			}
		}
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
		BoxShape2D* box = m_Pool.Allocate<BoxShape2D>(body, min, max);
		body->m_Fixtures.push_back({ box, density });
		body->recalculateMass();
		box->m_ID = m_Tree.Insert(body->m_ID, box->GetAABB());
		return box;
	}

	CircleShape* PhysicsWorld::AddCircleShape(PhysicsBody* body, const glm::vec2& offset, float radius, float density)
	{
		CircleShape* circle = m_Pool.Allocate<CircleShape>(body, offset, radius);
		body->m_Fixtures.push_back({ circle, density });
		body->recalculateMass();
		circle->m_ID = m_Tree.Insert(body->m_ID, circle->GetAABB() + offset);
		return circle;
	}
	void PhysicsWorld::broadPhase(Timestep ts)
	{
		// Find possible collisions between moving nodes
		auto& movedNodes = m_Tree.GetMovedNodes();
		int32_t counter = 0;
		for (auto node : movedNodes)
		{
			if (node)
			{
				auto func = [&](int32_t shapeID) -> bool
				{
					if (counter == shapeID)
						return false;

					if (movedNodes[shapeID] && shapeID > counter)
						return false;

					if (m_Tree.GetObjectIndex(shapeID) == m_Tree.GetObjectIndex(counter))
						return false;

					m_IntersectingNodes.push_back({ counter, shapeID });
					return false;
				};

				// Use fat aabb to prevent tunneling
				AABB fatAABB = m_Tree.GetAABB(counter);
				fatAABB.Min.x -= fabs(m_Bodies[m_Tree.GetObjectIndex(counter)]->m_Velocity.x) * ts;
				fatAABB.Min.y -= fabs(m_Bodies[m_Tree.GetObjectIndex(counter)]->m_Velocity.y) * ts;
				fatAABB.Max.x += fabs(m_Bodies[m_Tree.GetObjectIndex(counter)]->m_Velocity.x) * ts;
				fatAABB.Max.y += fabs(m_Bodies[m_Tree.GetObjectIndex(counter)]->m_Velocity.y) * ts;
				m_Tree.Query(func, fatAABB);
				SubmitBoxToRenderer(fatAABB, glm::vec4(1.0f, 0.0f, 0.0f, 1.0f));
			}
			counter++;
		}
	}
}