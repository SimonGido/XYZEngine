#include "stdafx.h"
#include "PhysicsShape.h"
#include "PhysicsBody.h"

#include <glm/ext/scalar_constants.hpp>

namespace XYZ {
	static bool TestAxis(const glm::vec2& axis, float minA, float maxA, float minB, float maxB, glm::vec2& mtvAxis, float& mtvDistance)
	{
		// Separating Axis Theorem
		// =======================
		// - Two convex shapes only overlap if they overlap on all axes of separation
		// - In order to create accurate responses we need to find the collision vector (Minimum Translation Vector)   
		// - The collision vector is made from a vector and a scalar, 
		//   - The vector value is the axis associated with the smallest penetration
		//   - The scalar value is the smallest penetration value
		// - Find if the two boxes intersect along a single axis
		// - Compute the intersection interval for that axis
		// - Keep the smallest intersection/penetration value
		float axisLengthSquared = glm::dot(axis, axis);

		// If the axis is degenerate then ignore
		if (axisLengthSquared < 1.0e-8f)
			return true;

		// Calculate the two possible overlap ranges
		// Either we overlap on the left or the right sides
		float d0 = (maxB - minA);   // 'Left' side
		float d1 = (maxA - minB);   // 'Right' side

		// Intervals do not overlap, so no intersection
		if (d0 <= 0.0f || d1 <= 0.0f)
			return false;

		// Find out if we overlap on the 'right' or 'left' of the object.
		float overlap = (d0 < d1) ? d0 : -d1;

		// The mtd vector for that axis
		glm::vec2 sep = axis * (overlap / axisLengthSquared);

		// The mtd vector length squared
		float sepLengthSquared = glm::dot(sep, sep);

		// If that vector is smaller than our computed Minimum Translation Distance use that vector as our current MTV distance
		if (sepLengthSquared < mtvDistance)
		{
			mtvDistance = sepLengthSquared;
			mtvAxis = sep;
		}

		return true;
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
	IntersectData BoxShape2D::Intersect(const PhysicsShape& shape) const
	{
		IntersectData data;
		if (shape.GetType() == ShapeType::Box)
		{
			const BoxShape2D& box = (const BoxShape2D&)shape;
			data.Intersection = box.GetAABB().Intersect(GetAABB());
			if (data.Intersection)
			{
				float mtvDistance = FLT_MAX;
				glm::vec2 mtvAxis(0.0f);

				if (!TestAxis(glm::vec2(1.0f, 0.0f), Min.x, Max.x, box.Min.x, box.Max.x, mtvAxis, mtvDistance))
					return data;

				if (!TestAxis(glm::vec2(0.0f, 1.0f), Min.y, Max.y, box.Min.y, box.Max.y, mtvAxis, mtvDistance))
					return data;

				data.Enter = glm::normalize(mtvAxis);
				data.Penetration = glm::sqrt(mtvDistance) * 1.001f;
			}
		}
		return data;
	}

	AABB BoxShape2D::GetAABB() const
	{
		{ return { {m_Body->GetPosition() + Min, 0.0f}, {m_Body->GetPosition() + Max,0.0f} }; }
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

	IntersectData CircleShape::Intersect(const PhysicsShape& shape) const
	{
		return IntersectData();
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