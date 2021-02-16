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
		body->m_InverseInertia = 1.0f / box->CalculateInertia(body->m_Mass);
		body->m_Shape->m_ID = m_Tree.Insert(body->m_ID, box->GetAABB());
		return box;
	}

	CircleShape* PhysicsWorld::AddCircleShape(PhysicsBody* body, const glm::vec2& offset, float radius, float density)
	{
		CircleShape* circle = m_Pool.Allocate<CircleShape>(body, offset, radius);
		body->m_Shape = circle;
		body->SetDensity(density);
		body->m_InverseInertia = 1.0f / circle->CalculateInertia(body->m_Mass);
		body->m_Shape->m_ID = m_Tree.Insert(body->m_ID, circle->GetAABB() + offset);
		return circle;
	}
	void PhysicsWorld::broadPhase(Timestep ts)
	{
		for (size_t i = 0; i < m_Bodies.size(); ++i)
		{
			PhysicsBody* A = m_Bodies[i];
			for (size_t j = i + 1; j < m_Bodies.size(); ++j)
			{
				PhysicsBody* B = m_Bodies[j];
				auto data = A->GetShape()->Intersect(*B->GetShape(), ts);
				if (data.Intersection)
				{
					Manifold manifold;
					manifold.A = A;
					manifold.B = B;
					m_Manifolds.push_back(manifold);
				}
			}
		}
	}
	void PhysicsWorld::narrowPhase(Timestep ts)
	{
		for (auto& manifold : m_Manifolds)
		{
			manifold.Initialize(m_Gravity, ts);
			manifold.ApplyImpulse();
			manifold.PositionalCorrection();
		}

		// Restart intersecting nodes and moved nodes
		m_Manifolds.clear();
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
					m_Tree.Move(body->m_Shape->m_ID, body->m_Position - body->m_OldPosition);
				}
			}
			body->m_Forces = glm::vec2(0.0f);
		}
	}
}