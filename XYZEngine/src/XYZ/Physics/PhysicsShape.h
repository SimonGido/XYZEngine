#pragma once

#include "XYZ/Utils/Math/AABB.h"
#include <glm/glm.hpp>

namespace XYZ {

	enum class ShapeType
	{
		Box,
		Circle,
		Polygon,
		NumTypes
	};

	class PhysicsBody;
	struct PhysicsShape
	{
		PhysicsShape(ShapeType type, PhysicsBody* body);

		virtual bool Intersect(const PhysicsShape& shape) const = 0;
		virtual AABB GetAABB() const = 0;
		virtual float CalculateMass(float density) const = 0;
		virtual float CalculateInertia(float mass) const = 0;
		virtual glm::vec2 CalculateCenter() const { return glm::vec2(0.0f); }

		const PhysicsBody* GetBody() const { return m_Body; }
		ShapeType GetType() const { return m_Type; };
		int32_t GetID() const { return m_ID; }
	private:
		const ShapeType m_Type;
		int32_t m_ID = -1;

	protected:
		glm::vec2 m_Offset = glm::vec2(0.0f);
		const PhysicsBody* m_Body;
		friend class PhysicsWorld;
	};

	struct BoxShape2D : public PhysicsShape
	{
		BoxShape2D(PhysicsBody* body);
		BoxShape2D(PhysicsBody* body, const glm::vec2& min, const glm::vec2& max);

		glm::vec2 Min;
		glm::vec2 Max;           

		virtual bool Intersect(const PhysicsShape& shape) const override;
		virtual AABB GetAABB() const override;
		virtual float CalculateMass(float density) const override;
		virtual float CalculateInertia(float mass) const override;
		virtual glm::vec2 CalculateCenter() const override;
	};

	struct CircleShape : public PhysicsShape
	{
		CircleShape(PhysicsBody* body);
		CircleShape(PhysicsBody* body, float radius);

		virtual bool Intersect(const PhysicsShape& shape) const override;
		virtual AABB GetAABB() const override;
		virtual float CalculateMass(float density) const override;
		virtual float CalculateInertia(float mass) const override;

		float Radius;
	};

	struct PolygonShape : public PhysicsShape
	{
		PolygonShape(PhysicsBody* body);
		PolygonShape(PhysicsBody* body, const std::vector<glm::vec2>& vertices);

		virtual bool Intersect(const PhysicsShape& shape) const override;
		virtual AABB GetAABB() const override;
		virtual float CalculateMass(float density) const override;
		virtual float CalculateInertia(float mass) const override;
		virtual glm::vec2 CalculateCenter() const override;

		std::vector<glm::vec2> Vertices;
	};
}
