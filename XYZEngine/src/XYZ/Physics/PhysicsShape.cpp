#include "stdafx.h"
#include "PhysicsShape.h"


namespace XYZ {
	PhysicsShape::PhysicsShape(ShapeType type)
		:
		m_Type(type)
	{
	}

	BoxShape2D::BoxShape2D()
		:
		PhysicsShape(ShapeType::Box),
		Min(glm::vec2(0.0f)),
		Max(glm::vec2(0.0f))
	{
	}
	BoxShape2D::BoxShape2D(const glm::vec2& min, const glm::vec2& max)
		:
		PhysicsShape(ShapeType::Box),
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

	CircleShape::CircleShape()
		: 
		PhysicsShape(ShapeType::Circle),
		Radius(1.0f)
	{
	}

	CircleShape::CircleShape(float radius)
		: 
		PhysicsShape(ShapeType::Circle),
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
}