#include "stdafx.h"
#include "PhysicsShape.h"
#include "PhysicsBody.h"


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

	static glm::vec2 GetSupport(const glm::vec2& dir, glm::vec2* vertices, uint32_t vertexCount)
	{
		float bestProjection = -FLT_MAX;
		glm::vec2 bestVertex = glm::vec2(0.0f);

		for (uint32_t i = 0; i < vertexCount; ++i)
		{
			glm::vec2 v = vertices[i];
			float projection = glm::dot(v, dir);

			if (projection > bestProjection)
			{
				bestVertex = v;
				bestProjection = projection;
			}
		}
		return bestVertex;
	}

	static float FindAxisLeastPenetration(
		glm::vec2& normal, const glm::vec2* aNormals,
		glm::vec2* aVertices, glm::vec2* bVertices, uint32_t aVertexCount, uint32_t bVertexCount
	)
	{
		float bestDistance = -FLT_MAX;
		
		for (uint32_t i = 0; i < aVertexCount; ++i)
		{
			// Retrieve a face normal from A
			glm::vec2 n = aNormals[i];
			// Retrieve support point from B along -n
			glm::vec2 s = GetSupport(-n, bVertices, bVertexCount);
			// Retrieve vertex on face from A, transform into
			// B's model space
			glm::vec2 v = aVertices[i];
			// Compute penetration distance (in B's model space)
			float d = glm::dot(n, s - v);
			// Store greatest distance
			if (d > bestDistance)
			{
				bestDistance = d;
				normal = aNormals[i];
			}
		}
		return bestDistance;
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
			
			SubmitBoxToRenderer(GetAABB(), glm::vec4(0.0f, 0.0f, 1.0f, 1.0f));
			SubmitBoxToRenderer(box.GetAABB(), glm::vec4(0.0f, 0.0f, 1.0f, 1.0f));

			glm::vec2 firstSize = GetAABB().Max - GetAABB().Min;
			glm::vec2 secondSize = box.GetAABB().Max - box.GetAABB().Min;
			glm::vec2 points1[4] = {
				GetAABB().Min, GetAABB().Min + glm::vec3(firstSize.x ,0.0f,0.0f),
				GetAABB().Max, GetAABB().Min + glm::vec3(0.0f, firstSize.y ,0.0f),
			};
			glm::vec2 points2[4] = {
				box.GetAABB().Min, box.GetAABB().Min + glm::vec3(secondSize.x ,0.0f,0.0f),
				box.GetAABB().Max, box.GetAABB().Min + glm::vec3(0.0f, secondSize.y ,0.0f),
			};

			static constexpr glm::vec2 normals[4] = {
				{0,-1},{1,0},{0,1},{-1,0}
			};
			glm::vec2 normal = glm::vec2(0.0f);
			float overlap = FindAxisLeastPenetration(normal, normals, points1, points2, 4, 4);
			if (overlap >= 0.0f)
				return data;
			data.Displacement = -normal * overlap;
			data.Normal = normal;
			data.Intersection = true;
			return data;
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