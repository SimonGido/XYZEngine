#include "stdafx.h"
#include "PhysicsWorld.h"

#include "XYZ/Scene/Components.h"
#include "XYZ/Renderer/Renderer2D.h"

#include "XYZ/Utils/Math/Math.h"

#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtx/matrix_decompose.hpp>

namespace XYZ {

	static void SubmitBoxToRenderer(const AABB& box, const glm::vec4& color)
	{
		Renderer2D::SubmitLine(glm::vec3(box.Min.x, box.Min.y, box.Max.z), glm::vec3(box.Max.x, box.Min.y, box.Max.z), color);
		Renderer2D::SubmitLine(glm::vec3(box.Max.x, box.Min.y, box.Max.z), glm::vec3(box.Max.x, box.Max.y, box.Max.z), color);
		Renderer2D::SubmitLine(glm::vec3(box.Max.x, box.Max.y, box.Max.z), glm::vec3(box.Min.x, box.Max.y, box.Max.z), color);
		Renderer2D::SubmitLine(glm::vec3(box.Min.x, box.Max.y, box.Max.z), glm::vec3(box.Min.x, box.Min.y, box.Max.z), color);
	}

	PhysicsWorld::PhysicsWorld(const glm::vec2& gravity)
		:
		m_Pool(10 * 1024),
		m_Gravity(gravity)
	{
	}

	void PhysicsWorld::Update(Timestep ts)
	{
		m_DeltaTime = ts;
		broadPhase();
		applyForces();
		narrowPhase();
		
		applyVelocities();

		// Restart intersecting nodes and moved nodes
		m_Manifolds.clear();
		m_Tree.CleanMovedNodes();
	}

	void PhysicsWorld::DestroyBody(PhysicsBody* body)
	{
		m_Tree.Remove(body->m_Shape->m_ID);
		auto & it = std::find_if(m_Bodies.begin(), m_Bodies.end(), [body](const PhysicsBody* a) {
			return a->GetID() == body->m_ID;
		});
		if (it != m_Bodies.end())
			m_Bodies.erase(it);

		m_Pool.Deallocate<PhysicsBody>(body);
	}

	void PhysicsWorld::DestroyShape(PhysicsShape* shape)
	{
		m_Pool.Deallocate<PhysicsShape>(shape);
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
		if (body->m_Type == PhysicsBody::Type::Static)
			body->m_InverseInertia = 0.0f;
		body->m_Shape->m_ID = m_Tree.Insert(body->m_ID, box->GetAABB());
		return box;
	}

	CircleShape* PhysicsWorld::AddCircleShape(PhysicsBody* body, const float radius, float density)
	{
		CircleShape* circle = m_Pool.Allocate<CircleShape>(body, radius);
		body->m_Shape = circle;
		body->SetDensity(density);
		body->m_InverseInertia = 1.0f / circle->CalculateInertia(body->m_Mass);
		if (body->m_Type == PhysicsBody::Type::Static)
			body->m_InverseInertia = 0.0f;
		body->m_Shape->m_ID = m_Tree.Insert(body->m_ID, circle->GetAABB());
		return circle;
	}
	PolygonShape* PhysicsWorld::AddPolygonShape(PhysicsBody* body, const std::vector<glm::vec2>& vertices, float density)
	{
		PolygonShape* polygon = m_Pool.Allocate<PolygonShape>(body, vertices);
		body->m_Shape = polygon;
		body->SetDensity(density);
		body->m_InverseInertia = 1.0f / polygon->CalculateInertia(body->m_Mass);
		if (body->m_Type == PhysicsBody::Type::Static)
			body->m_InverseInertia = 0.0f;
		body->m_Shape->m_ID = m_Tree.Insert(body->m_ID, polygon->GetAABB());
		return nullptr;
	}
	void PhysicsWorld::broadPhase()
	{
		for (size_t i = 0; i < m_Bodies.size(); ++i)
		{
			PhysicsBody* A = m_Bodies[i];
			for (size_t j = i + 1; j < m_Bodies.size(); ++j)
			{
				PhysicsBody* B = m_Bodies[j];
				if (A->GetShape()->Intersect(*B->GetShape()))
				{
					Manifold manifold;
					manifold.A = A;
					manifold.B = B;
					m_Manifolds.push_back(manifold);
				}
			}
		}
	}

	void PhysicsWorld::applyForces()
	{
		for (auto& body : m_Bodies)
		{
			if (body->m_Type != PhysicsBody::Type::Static)
			{
				body->m_OldPosition = body->m_Position;
				body->m_Velocity += (m_Gravity + (body->m_Forces / body->m_Mass)) * m_DeltaTime;
			}
			body->m_Forces = glm::vec2(0.0f);
			body->m_Torque = 0.0f;
		}
	}

	void PhysicsWorld::applyVelocities()
	{
		for (auto& body : m_Bodies)
		{
			if (body->m_Type != PhysicsBody::Type::Static)
			{
				body->m_Position += body->m_Velocity * m_DeltaTime;
				body->m_Angle += body->m_AngularVelocity * m_DeltaTime;
				if (body->m_Position != body->m_OldPosition)
					m_Tree.Move(body->m_Shape->m_ID, body->m_Position - body->m_OldPosition);
			}
		}
	}

	void PhysicsWorld::narrowPhase()
	{
		for (auto& manifold : m_Manifolds)
			manifold.Initialize(m_Gravity, m_DeltaTime);
		for (auto& manifold : m_Manifolds)
			manifold.ApplyImpulse();
		for (auto& manifold : m_Manifolds)
			manifold.PositionalCorrection();	

		debugDraw();
	}
	void PhysicsWorld::debugDraw()
	{
		glm::vec4 centerColor = glm::vec4(1.0f, 0.0f, 1.0f, 1.0f);
		glm::vec4 velocityColor = glm::vec4(0.0f, 1.0f, 0.0f, 1.0f);
		glm::vec4 contactColor = glm::vec4(1.0f, 0.0f, 1.0f, 1.0f);
		glm::vec4 shapeColor = glm::vec4(0.0f, 1.0f, 1.0f, 1.0f);
		for (auto body : m_Bodies)
		{
			glm::vec2 vel = glm::normalize(body->m_Velocity);
			Renderer2D::SubmitCircle(glm::vec3(body->m_Shape->CalculateCenter(), 0.0f), 0.5f, 10, centerColor);
			Renderer2D::SubmitLine(glm::vec3(body->m_Position, 0.0f), glm::vec3(body->m_Position + vel, 0.0f), velocityColor);

			if (body->m_Shape->m_Type == ShapeType::Box)
			{
				SubmitBoxToRenderer(body->m_Shape->GetAABB(), shapeColor);
			}
			else if (body->m_Shape->m_Type == ShapeType::Circle)
			{
				Renderer2D::SubmitCircle(glm::vec3(body->m_Position, 0.0f),
					static_cast<CircleShape*>(body->m_Shape)->Radius, 15, shapeColor);
			}
		}

		for (auto& manifold : m_Manifolds)
		{
			for (uint32_t i = 0; i < manifold.ContactCount; ++i)
				Renderer2D::SubmitCircle(glm::vec3(manifold.Contacts[i], 0.0f), 0.5f, 10, contactColor);
		}

		m_Tree.SubmitToRenderer();
	}
}