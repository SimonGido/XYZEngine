#include "stdafx.h"
#include "PhysicsShape.h"
#include "PhysicsBody.h"

#include "XYZ/Utils/Math/Math.h"

#include "XYZ/Renderer/Renderer2D.h"

#include <glm/ext/scalar_constants.hpp>

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
	
	static bool CircleIntersectsCircle(float x1, float y1, float r1, float x2, float y2, float r2)
	{
		return fabs((x1 - x2) * (x1 - x2) + (y1 - y2) * (y1 - y2)) <= (r1 + r2) * (r1 + r2);
	};


	static bool RectangleIntersectsRectangle(const glm::vec2& minA, const glm::vec2& maxA, const glm::vec2& minB, const glm::vec2& maxB)
	{
		return (minA.x < maxB.x&& maxA.x > minB.x && minA.y < maxB.y && maxA.y > minB.y);
	}

	PhysicsShape::PhysicsShape(ShapeType type, PhysicsBody* body)
		:
		m_Type(type),
		m_Body(body)
	{
	}

	BoxShape2D::BoxShape2D(PhysicsBody* body)
		:
		PhysicsShape(ShapeType::Box, body),
		Min(glm::vec2(0.0f)),
		Max(glm::vec2(0.0f))
	{
	}
	BoxShape2D::BoxShape2D(PhysicsBody* body, const glm::vec2& min, const glm::vec2& max)
		:
		PhysicsShape(ShapeType::Box, body),
		Min(min),
		Max(max)
	{
	}
	bool BoxShape2D::Intersect(const PhysicsShape& shape) const
	{
		if (shape.GetType() == ShapeType::Box)
		{
			const BoxShape2D& box = (const BoxShape2D&)shape;
			const AABB& otherAABB = box.GetAABB();
			const AABB& aabb = GetAABB();

			return RectangleIntersectsRectangle(aabb.Min, aabb.Max, otherAABB.Min, otherAABB.Max);
		}
		return false;
	}

	AABB BoxShape2D::GetAABB() const
	{
		 return { {m_Body->GetPosition() + Min, 0.0f}, {m_Body->GetPosition() + Max,0.0f} };
	}

	
	float BoxShape2D::CalculateMass(float density) const
	{
		float a = Max.x - Min.x;
		float b = Max.y - Min.y;
		return a * b * density;
	}

	float BoxShape2D::CalculateInertia(float mass) const
	{
		float a = Max.x - Min.x;
		float b = Max.y - Min.y;
		return mass * (a * a + b * b) / 12.0f;
	}

	float BoxShape2D::CalculateTorque(const glm::vec2& force, const glm::vec2& pos) const
	{
		float centerX = (Max.x - Min.x) / 2.0f;
		float centerY = (Max.y - Min.y) / 2.0f;
		glm::vec2 d = glm::vec2(pos.x - centerX, pos.y - centerY);

		return d.x * force.y - force.x * d.y;
	}

	glm::vec2 BoxShape2D::CalculateCenter() const
	{
		float centerX = (Max.x - Min.x) / 2.0f;
		float centerY = (Max.y - Min.y) / 2.0f;
		return { centerX, centerY };
	}

	CircleShape::CircleShape(PhysicsBody* body)
		: 
		PhysicsShape(ShapeType::Circle, body),
		Radius(1.0f)
	{
	}

	CircleShape::CircleShape(PhysicsBody* body, const glm::vec2& offset,float radius)
		: 
		PhysicsShape(ShapeType::Circle, body),
		Radius(radius)
	{
		m_Offset = offset;
	}

	bool CircleShape::Intersect(const PhysicsShape& shape) const
	{
		if (shape.GetType() == ShapeType::Circle)
		{
			const CircleShape& ball = (CircleShape&)shape;
			return CircleIntersectsCircle(
				m_Body->GetPosition().x + m_Offset.x, m_Body->GetPosition().y + m_Offset.y, Radius,
				ball.m_Body->GetPosition().x + ball.m_Offset.x, ball.m_Body->GetPosition().y + ball.m_Offset.y, ball.Radius
			);	
		}
		return false;
	}

	AABB CircleShape::GetAABB() const
	{
		return { 
			glm::vec3(m_Body->GetPosition().x - Radius, m_Body->GetPosition().y - Radius, 0.0f), 
			glm::vec3(m_Body->GetPosition().x + Radius, m_Body->GetPosition().y + Radius, 0.0f)
		};
	}

	float CircleShape::CalculateMass(float density) const
	{
		return glm::pi<float>() * (Radius * Radius) * density;
	}
	float CircleShape::CalculateInertia(float mass) const
	{
		return 0.0f;
	}
	float CircleShape::CalculateTorque(const glm::vec2& force, const glm::vec2& pos) const
	{
		return 0.0f;
	}
}