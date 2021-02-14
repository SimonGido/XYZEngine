#include "stdafx.h"
#include "PhysicsShape.h"
#include "PhysicsBody.h"


#include "XYZ/Renderer/Renderer2D.h"

#include <glm/ext/scalar_constants.hpp>

namespace XYZ {
	
	static bool CircleIntersectsCircle(float x1, float y1, float r1, float x2, float y2, float r2)
	{
		return fabs((x1 - x2) * (x1 - x2) + (y1 - y2) * (y1 - y2)) <= (r1 + r2) * (r1 + r2);
	};

	//static IntersectData RayIntersectsAABB(const glm::vec2& origin, const glm::vec2& dir, const AABB& target)
	//{
	//	IntersectData result;
	//	glm::vec2 targetSize = target.Max - target.Min;
	//	glm::vec2 min = { target.Min.x, target.Min.y };
	//	glm::vec2 max = { target.Max.x, target.Max.y };
	//	glm::vec2 tNear = (min - origin) / dir;
	//	glm::vec2 tFar = glm::vec2(max - origin) / dir;
	//	
	//	if (std::isnan(tFar.y) || std::isnan(tFar.x)) return result;
	//	if (std::isnan(tNear.y) || std::isnan(tNear.x)) return result;
	//
	//	if (tNear.x > tFar.x) std::swap(tNear.x, tFar.x);
	//	if (tNear.y > tFar.y) std::swap(tNear.y, tFar.y);
	//	if (tNear.x > tFar.y || tNear.y > tFar.x) return result;
	//
	//	result.HitTime = std::max(tNear.x, tNear.y);
	//	float tHitFar = std::min(tFar.x, tFar.y);
	//
	//	if (result.HitTime > 1.0f) return result;
	//	if (tHitFar <= 0.0f) return result;
	//	
	//	result.ContactPoint = origin + result.HitTime * dir;
	//	if (tNear.x > tNear.y)
	//	{
	//		if (dir.x < 0.0f)
	//			result.ContactNormal = { 1.0f, 0.0f };
	//		else
	//			result.ContactNormal = { -1.0f, 0.0f };
	//	}
	//	else if (tNear.x < tNear.y)
	//	{
	//		if (dir.y < 0.0f)
	//			result.ContactNormal = { 0.0f, 1.0f };
	//		else
	//			result.ContactNormal = { 0.0f, -1.0f };
	//	}
	//	result.Hit = true;
	//	return result;
	//}
	
	static IntersectData DynamicAABBIntersectsAABB(const AABB& in, const AABB& target, const glm::vec2& velocity, float time)
	{
		//AABB expanded;
		//glm::vec3 inSize = in.Max - in.Min;
		//expanded.Min = target.Min - inSize / 2.0f;
		//expanded.Max = target.Max + inSize / 2.0f;
		//
		//IntersectData result = RayIntersectsAABB(in.Min + inSize / 2.0f, velocity * time, expanded);
		//
		//return result;
		
		return IntersectData();
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
	IntersectData BoxShape2D::Intersect(const PhysicsShape& shape, float time) const
	{
		IntersectData data;
		if (shape.GetType() == ShapeType::Box)
		{
			const BoxShape2D& box = (const BoxShape2D&)shape;
			//return DynamicAABBIntersectsAABB(GetAABB(), box.GetAABB(), m_Body->GetVelocity(), time);
		}
		return data;
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

	IntersectData CircleShape::Intersect(const PhysicsShape& shape, float time) const
	{
		IntersectData data;
		if (shape.GetType() == ShapeType::Circle)
		{
			const CircleShape& ball = (CircleShape&)shape;
			Renderer2D::SubmitCircle(glm::vec3(m_Body->m_Position + m_Offset, 0.0f), Radius, glm::vec4(0.0f, 1.0f, 0.0f, 1.0f));
			Renderer2D::SubmitCircle(glm::vec3(ball.m_Body->m_Position + ball.m_Offset, 0.0f), ball.Radius, glm::vec4(0.0f, 1.0f, 0.0f, 1.0f));
			
			if (CircleIntersectsCircle(
				m_Body->m_Position.x + m_Offset.x, m_Body->m_Position.y + m_Offset.y, Radius,
				ball.m_Body->m_Position.x + ball.m_Offset.x, ball.m_Body->m_Position.y + ball.m_Offset.y, ball.Radius
			))
			{
				float distance = sqrtf(
					  ((m_Body->m_Position.x + m_Offset.x) - (ball.m_Body->m_Position.x + ball.m_Offset.x))
					* ((m_Body->m_Position.x + m_Offset.x) - (ball.m_Body->m_Position.x + ball.m_Offset.x))
					+ ((m_Body->m_Position.y + m_Offset.y) - (ball.m_Body->m_Position.y + ball.m_Offset.y))
					* ((m_Body->m_Position.y + m_Offset.y) - (ball.m_Body->m_Position.y + ball.m_Offset.y))
				);
				float overlap = 0.5f * (distance - Radius - ball.Radius);
				data.Intersection = true;
				data.Displacement = overlap * ((m_Body->m_Position + m_Offset) - (ball.m_Body->m_Position + ball.m_Offset)) / distance;
			}
		}
		return data;
	}

	AABB CircleShape::GetAABB() const
	{
		return { 
			glm::vec3(m_Body->m_Position.x - Radius, m_Body->m_Position.y - Radius, 0.0f), 
			glm::vec3(m_Body->m_Position.x + Radius, m_Body->m_Position.y + Radius, 0.0f)
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