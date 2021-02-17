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

	glm::vec2 BoxShape2D::CalculateCenter() const
	{
		float centerX = Min.x + (Max.x - Min.x) / 2.0f;
		float centerY = Min.y + (Max.y - Min.y) / 2.0f;
		return { m_Body->GetPosition().x + centerX, m_Body->GetPosition().y + centerY };
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
		return (glm::pi<float>() * (Radius * Radius * Radius * Radius)) / 4.0f;
	}
	
	PolygonShape::PolygonShape(PhysicsBody* body)
		:
		PhysicsShape(ShapeType::Polygon, body)
	{
	}
	PolygonShape::PolygonShape(PhysicsBody* body, const std::vector<glm::vec2>& vertices)
		:
		PhysicsShape(ShapeType::Polygon, body),
		Vertices(vertices)
	{
	}
	bool PolygonShape::Intersect(const PhysicsShape& shape) const
	{
		return false;
	}
	AABB PolygonShape::GetAABB() const
	{
		AABB box;
		box.Min = glm::vec3(FLT_MAX, FLT_MAX, 0.0f);
		box.Max = glm::vec3(-FLT_MAX, -FLT_MAX, 0.0f);
		for (auto& vertex : Vertices)
		{
			if (box.Min.x > vertex.x)
				box.Min.x = vertex.x;
			if (box.Min.y > vertex.y)
				box.Min.y = vertex.y;
			if (box.Max.x < vertex.x)
				box.Max.x = vertex.x;
			if (box.Max.y < vertex.y)
				box.Max.y = vertex.y;
		}
		auto& bodyPos = m_Body->GetPosition();
		box.Min.x += bodyPos.x;
		box.Min.y += bodyPos.y;
		box.Max.x += bodyPos.x;
		box.Max.y += bodyPos.y;
		return box;
	}
	float PolygonShape::CalculateMass(float density) const
	{
		// Calculate centroid and moment of interia
		glm::vec2 centroid(0.0f, 0.0f); // centroid
		float area = 0.0f;
		float inertia = 0.0f;
		const float inv3 = 1.0f / 3.0f;

		for (size_t i = 0; i < Vertices.size(); ++i)
		{
			// Triangle vertices, third vertex implied as (0, 0)
			glm::vec2 p1(Vertices[i]);
			glm::vec2 p2 = Vertices[0];
			if (i < Vertices.size() - 1)
				p2 = Vertices[i + 1];
				
			float d = Math::Cross(p1, p2);
			float triangleArea = 0.5f * d;

			area += triangleArea;

			// Use area to weight the centroid average, not just vertex position
			centroid += triangleArea * inv3 * (p1 + p2);

			float intx2 = p1.x * p1.x + p2.x * p1.x + p2.x * p2.x;
			float inty2 = p1.y * p1.y + p2.y * p1.y + p2.y * p2.y;
			inertia += (0.25f * inv3 * d) * (intx2 + inty2);
		}
		return density * area;
	}
	float PolygonShape::CalculateInertia(float mass) const
	{
		glm::vec2 centroid(0.0f, 0.0f); // centroid
		float area = 0.0f;
		float inertia = 0.0f;
		const float inv3 = 1.0f / 3.0f;

		for (size_t i = 0; i < Vertices.size(); ++i)
		{
			// Triangle vertices, third vertex implied as (0, 0)
			glm::vec2 p1(Vertices[i]);
			glm::vec2 p2 = Vertices[0];
			if (i < Vertices.size() - 1)
				p2 = Vertices[i + 1];

			float d = Math::Cross(p1, p2);
			float triangleArea = 0.5f * d;

			area += triangleArea;

			// Use area to weight the centroid average, not just vertex position
			centroid += triangleArea * inv3 * (p1 + p2);

			float intx2 = p1.x * p1.x + p2.x * p1.x + p2.x * p2.x;
			float inty2 = p1.y * p1.y + p2.y * p1.y + p2.y * p2.y;
			inertia += (0.25f * inv3 * d) * (intx2 + inty2);
		}
		return inertia * area * mass;
	}

	glm::vec2 PolygonShape::CalculateCenter() const
	{
		glm::vec2 centroid(0.0f, 0.0f);
		const float inv3 = 1.0f / 3.0f;
		for (size_t i = 0; i < Vertices.size(); ++i)
		{
			// Triangle vertices, third vertex implied as (0, 0)
			glm::vec2 p1(Vertices[i]);
			glm::vec2 p2 = Vertices[0];
			if (i < Vertices.size() - 1)
				p2 = Vertices[i + 1];

			float d = Math::Cross(p1, p2);
			float triangleArea = 0.5f * d;
		
			// Use area to weight the centroid average, not just vertex position
			centroid += triangleArea * inv3 * (p1 + p2);
		}
		return m_Body->GetPosition() + centroid;
	}
}