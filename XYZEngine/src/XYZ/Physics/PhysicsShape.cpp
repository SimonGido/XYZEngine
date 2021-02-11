#include "stdafx.h"
#include "PhysicsShape.h"

#include <glm/ext/scalar_constants.hpp>

namespace XYZ {
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
			return box.GetAABB().Intersect(GetAABB());
		}
		return false;
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

	CircleShape::CircleShape(PhysicsBody* body,float radius)
		: 
		PhysicsShape(ShapeType::Circle, body),
		Radius(radius)
	{
	}

	bool CircleShape::Intersect(const PhysicsShape& shape) const
	{
		return false;
	}

	AABB CircleShape::GetAABB() const
	{
		return { glm::vec3(-Radius), glm::vec3(Radius) };
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