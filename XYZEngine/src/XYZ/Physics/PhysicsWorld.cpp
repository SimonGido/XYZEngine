#include "stdafx.h"
#include "PhysicsWorld.h"

#include "XYZ/Scene/Components.h"
#include "XYZ/Renderer/Renderer2D.h"

namespace XYZ {

	static void SubmitBoxToRenderer(const AABB& box)
	{
		Renderer2D::SubmitLine(box.Min, glm::vec3(box.Max.x, box.Min.y, box.Min.z));
		Renderer2D::SubmitLine(glm::vec3(box.Max.x, box.Min.y, box.Min.z), glm::vec3(box.Max.x, box.Max.y, box.Min.z));
		Renderer2D::SubmitLine(glm::vec3(box.Max.x, box.Max.y, box.Min.z), glm::vec3(box.Min.x, box.Max.y, box.Min.z));
		Renderer2D::SubmitLine(glm::vec3(box.Min.x, box.Max.y, box.Min.z), box.Min);


		Renderer2D::SubmitLine(glm::vec3(box.Min.x, box.Min.y, box.Max.z), glm::vec3(box.Max.x, box.Min.y, box.Max.z));
		Renderer2D::SubmitLine(glm::vec3(box.Max.x, box.Min.y, box.Max.z), glm::vec3(box.Max.x, box.Max.y, box.Max.z));
		Renderer2D::SubmitLine(glm::vec3(box.Max.x, box.Max.y, box.Max.z), glm::vec3(box.Min.x, box.Max.y, box.Max.z));
		Renderer2D::SubmitLine(glm::vec3(box.Min.x, box.Max.y, box.Max.z), glm::vec3(box.Min.x, box.Min.y, box.Max.z));


		Renderer2D::SubmitLine(box.Min, glm::vec3(box.Min.x, box.Min.y, box.Max.z));
		Renderer2D::SubmitLine(glm::vec3(box.Min.x, box.Max.y, box.Min.z), glm::vec3(box.Min.x, box.Max.y, box.Max.z));


		Renderer2D::SubmitLine(glm::vec3(box.Max.x, box.Min.y, box.Min.z), glm::vec3(box.Max.x, box.Min.y, box.Max.z));
		Renderer2D::SubmitLine(glm::vec3(box.Max.x, box.Max.y, box.Min.z), glm::vec3(box.Max.x, box.Max.y, box.Max.z));

	}

	//if (shapeID != fixture.Shape->GetID())
						//{
						//	PhysicsBody* otherBody = m_Bodies[bodyID];
						//	auto data = fixture.Shape->Intersect(*otherBody->GetFixtures()[0].Shape, updateFrequency);
						//	if (data.Hit)
						//	{
						//		velocity += data.ContactNormal
						//			* glm::vec2(fabs(velocity.x), fabs(velocity.y)) * (1.0f - data.HitTime);
						//
						//
						//		Renderer2D::SubmitLine(glm::vec3(data.ContactPoint, 0.0f), glm::vec3(data.ContactNormal, 0.0f));
						//	}
						//	return false;
						//}



	PhysicsWorld::PhysicsWorld(const glm::vec2& gravity)
		:
		m_Pool(10 * 1024),
		m_Gravity(gravity)
	{
	}

	void PhysicsWorld::Update(Timestep ts, float updateFrequency)
	{
		if (m_CurrentTime >= updateFrequency)
		{
			m_CurrentTime = 0.0f;
			

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

						m_IntersectingNodes.push_back({ counter, shapeID });
						return false;
					};
					
					// Use fat aabb to prevent tunneling
					AABB fatAABB = m_Tree.GetAABB(counter);
					fatAABB.Min.x -= fabs(m_Bodies[m_Tree.GetObjectIndex(counter)]->m_LinearVelocity.x) * updateFrequency;
					fatAABB.Min.y -= fabs(m_Bodies[m_Tree.GetObjectIndex(counter)]->m_LinearVelocity.y) * updateFrequency;
					fatAABB.Max.x += fabs(m_Bodies[m_Tree.GetObjectIndex(counter)]->m_LinearVelocity.x) * updateFrequency;
					fatAABB.Max.y += fabs(m_Bodies[m_Tree.GetObjectIndex(counter)]->m_LinearVelocity.y) * updateFrequency;
					m_Tree.Query(func, fatAABB);
					SubmitBoxToRenderer(fatAABB);
				}
				counter++;
			}

			// Go through possible intersections and resolve collisions
			for (auto& it : m_IntersectingNodes)
			{
				auto firstBody = m_Bodies[m_Tree.GetObjectIndex(it.first)];
				auto secondBody = m_Bodies[m_Tree.GetObjectIndex(it.second)];

				for (auto& fixture : firstBody->GetFixtures())
				{
					for (auto& otherFixture : secondBody->GetFixtures())
					{
						// First body is moving for sure
						{
							auto data = fixture.Shape->Intersect(*otherFixture.Shape, updateFrequency);
							if (data.Hit)
							{
								firstBody->m_LinearVelocity += data.ContactNormal
									* glm::vec2(fabs(firstBody->m_LinearVelocity.x), fabs(firstBody->m_LinearVelocity.y)) * (1.0f - data.HitTime);
							}
						}
						// Check if second body is also moving, to prevent division by zero ( velocity )
						{
							auto data = otherFixture.Shape->Intersect(*fixture.Shape, updateFrequency);
							if (data.Hit && movedNodes[otherFixture.Shape->GetID()])
							{
								secondBody->m_LinearVelocity += data.ContactNormal
									* glm::vec2(fabs(secondBody->m_LinearVelocity.x), fabs(secondBody->m_LinearVelocity.y)) * (1.0f - data.HitTime);
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
					body->m_OldPosition = body->m_Position;
					glm::vec2 forces = m_Gravity;
					body->m_Acceleration = forces;
					body->m_LinearVelocity += body->m_Acceleration * updateFrequency;
					body->m_Position += body->m_LinearVelocity * updateFrequency;
					if (body->m_Position != body->m_OldPosition)
					{
						for (auto& fixture : body->m_Fixtures)
						{
							m_Tree.Move(fixture.Shape->m_ID, body->m_Position - body->m_OldPosition);
						}
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
		BoxShape2D* box = m_Pool.Allocate<BoxShape2D>(body, min, max);
		body->m_Fixtures.push_back({ box, density });
		body->recalculateMass();
		box->m_ID = m_Tree.Insert(body->m_ID, box->GetAABB());
		return box;
	}

	CircleShape* PhysicsWorld::AddCircleShape(PhysicsBody* body, float radius, float density)
	{
		CircleShape* circle = m_Pool.Allocate<CircleShape>(body, radius);
		body->m_Fixtures.push_back({ circle, density });
		body->recalculateMass();
		circle->m_ID = m_Tree.Insert(body->m_ID, circle->GetAABB());
		return circle;
	}
}