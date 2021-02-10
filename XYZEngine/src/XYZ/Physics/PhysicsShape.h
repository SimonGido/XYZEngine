#pragma once

#include "XYZ/Utils/Math/AABB.h"
#include <glm/glm.hpp>

namespace XYZ {

	enum class ShapeType
	{
		Box,
		Circle,
		Chain,
		NumTypes
	};

	struct PhysicsShape
	{
		PhysicsShape(ShapeType type);

		virtual bool Intersect(const PhysicsShape& shape) const = 0;
		virtual AABB GetAABB() const = 0;
		ShapeType GetType() const { return m_Type; };

	private:
		const ShapeType m_Type;
		int32_t m_ID = -1;

		friend class PhysicsWorld;
	};

	struct BoxShape2D : public PhysicsShape
	{
		BoxShape2D();
		BoxShape2D(const glm::vec2& min, const glm::vec2& max);

		glm::vec2 Min;
		glm::vec2 Max;           

		virtual bool Intersect(const PhysicsShape& shape) const override;

		virtual AABB GetAABB() const override { return { {Min, 0.0f}, {Max,0.0f} }; }
	};

	struct CircleShape : public PhysicsShape
	{
		CircleShape();
		CircleShape(float radius);

		virtual bool Intersect(const PhysicsShape& shape) const override;

		virtual AABB GetAABB() const override;

		float Radius;
	};
}
