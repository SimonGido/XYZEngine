#include "stdafx.h"
#include "PhysicsDebugRenderer.h"

#include "Renderer2D.h"

namespace XYZ {
	void PhysicsDebugRenderer::DrawPolygon(const b2Vec2* vertices, int32 vertexCount, const b2Color& color)
	{
		for (int32_t i = 1; i < vertexCount; ++i)
		{
			glm::vec3 p0 = glm::vec3(vertices[i - 1].x, vertices[i - 1].y, 0.0f);
			glm::vec3 p1 = glm::vec3(vertices[i].x, vertices[i].y, 0.0f);
			Renderer2D::SubmitLine(p0, p1, glm::vec4(color.r, color.g, color.b, color.a));
		}
		glm::vec3 p0 = glm::vec3(vertices[0].x, vertices[0].y, 0.0f);
		glm::vec3 p1 = glm::vec3(vertices[vertexCount - 1].x, vertices[vertexCount - 1].y, 0.0f);
		Renderer2D::SubmitLine(p0, p1, glm::vec4(color.r, color.g, color.b, color.a));
	}
	void PhysicsDebugRenderer::DrawSolidPolygon(const b2Vec2* vertices, int32 vertexCount, const b2Color& color)
	{
		for (int32_t i = 1; i < vertexCount; ++i)
		{
			glm::vec3 p0 = glm::vec3(vertices[i - 1].x, vertices[i - 1].y, 0.0f);
			glm::vec3 p1 = glm::vec3(vertices[i].x, vertices[i].y, 0.0f);
			Renderer2D::SubmitLine(p0, p1, glm::vec4(color.r, color.g, color.b, color.a));
		}
		glm::vec3 p0 = glm::vec3(vertices[0].x, vertices[0].y, 0.0f);
		glm::vec3 p1 = glm::vec3(vertices[vertexCount - 1].x, vertices[vertexCount - 1].y, 0.0f);
		Renderer2D::SubmitLine(p0, p1, glm::vec4(color.r, color.g, color.b, color.a));
	}
	void PhysicsDebugRenderer::DrawCircle(const b2Vec2& center, float radius, const b2Color& color)
	{
		
	}
	void PhysicsDebugRenderer::DrawSolidCircle(const b2Vec2& center, float radius, const b2Vec2& axis, const b2Color& color)
	{
		
	}
	void PhysicsDebugRenderer::DrawSegment(const b2Vec2& p1, const b2Vec2& p2, const b2Color& color)
	{
		
	}
	void PhysicsDebugRenderer::DrawTransform(const b2Transform& xf)
	{
		
	}
	void PhysicsDebugRenderer::DrawPoint(const b2Vec2& p, float size, const b2Color& color)
	{
		
	}
}