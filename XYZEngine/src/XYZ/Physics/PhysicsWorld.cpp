#include "stdafx.h"
#include "PhysicsWorld.h"

#include "XYZ/Scene/Components.h"
#include "XYZ/Renderer/Renderer2D.h"


#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtx/matrix_decompose.hpp>

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
		for (auto& body : m_Bodies)
		{
			body->m_OldPosition = body->m_Position;
		}
		broadPhase(ts);
		narrowPhase(ts);

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
		body->m_Shape = box;
		body->SetDensity(density);
		body->m_Shape->m_ID = m_Tree.Insert(body->m_ID, box->GetAABB());
		return box;
	}

	CircleShape* PhysicsWorld::AddCircleShape(PhysicsBody* body, const glm::vec2& offset, float radius, float density)
	{
		CircleShape* circle = m_Pool.Allocate<CircleShape>(body, offset, radius);
		body->m_Shape = circle;
		body->SetDensity(density);
		body->m_Shape->m_ID = m_Tree.Insert(body->m_ID, circle->GetAABB() + offset);
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
				auto func = [&](uint32_t bodyIndex) -> bool
				{
					if (counter == bodyIndex)
						return false;

					if (movedNodes[bodyIndex] && bodyIndex > counter)
						return false;

					
					Manifold manifold;
					manifold.A = m_Bodies[counter];
					manifold.B = m_Bodies[bodyIndex];
					m_Manifolds.push_back(manifold);

					return false;
				};
				m_Tree.Query(func, m_Tree.GetAABB(counter));
			}
			counter++;
		}
	}
	void PhysicsWorld::narrowPhase(Timestep ts)
	{
		for (auto& manifold : m_Manifolds)
		{

		}

		// Restart intersecting nodes and moved nodes
		m_Manifolds.clear();
		m_Tree.CleanMovedNodes();

		// Apply velocity and forces
		for (auto& body : m_Bodies)
		{
			if (body->m_Type != PhysicsBody::Type::Static)
			{
				body->m_Velocity += m_Gravity + (body->m_Forces / body->m_Mass) * (float)ts;
				body->m_Position += body->m_Velocity * (float)ts;
				if (body->m_Position != body->m_OldPosition)
				{
					m_Tree.Move(body->m_Shape->m_ID, body->m_Position - body->m_OldPosition);
				}
			}
		}
	}
}