#pragma once

#include <glm/glm.hpp>


namespace XYZ {

	namespace Math {

		std::pair<glm::vec2, glm::vec2> CalculateVelocitiesAfterContact(
			const glm::vec2& firstVelocity, const glm::vec2& secondVelocity, float firstMass, float secondMass
		);

		glm::vec2 CalculateMirrorVector(const glm::vec2& dir, const glm::vec2& hitNormal);

		float PythagoreanSolve(float a, float b);

		void DecomposeTransform(const glm::mat4& transform, glm::vec3& translation, glm::vec3& rotation, glm::vec3& scale);

		inline glm::vec2 Cross(float a, const glm::vec2& v)
		{
			return glm::vec2(-a * v.y, a * v.x);
		}

		inline float Cross(const glm::vec2& a, const glm::vec2& b)
		{
			return a.x * b.y - a.y * b.x;
		}


		inline float Dot(const glm::vec2& a, const glm::vec2& b)
		{
			return a.x * b.x + a.y * b.y;
		}

		inline void Normalize(glm::vec2& a)
		{
			float len = std::sqrt(a.x * a.x + a.y * a.y);;

			if (len > FLT_EPSILON)
			{
				float invLen = 1.0f / len;
				a.x *= invLen;
				a.y *= invLen;
			}
		}

		inline float Sign(const glm::vec2& p1, const glm::vec2& p2, const glm::vec2& p3)
		{
			return (p1.x - p3.x) * (p2.y - p3.y) - (p2.x - p3.x) * (p1.y - p3.y);
		}

		inline bool PointInTriangle(const glm::vec2& pt, const glm::vec2& v1, const glm::vec2& v2, const glm::vec2& v3)
		{
			float d1, d2, d3;
			bool has_neg, has_pos;

			d1 = Sign(pt, v1, v2);
			d2 = Sign(pt, v2, v3);
			d3 = Sign(pt, v3, v1);

			has_neg = (d1 < 0) || (d2 < 0) || (d3 < 0);
			has_pos = (d1 > 0) || (d2 > 0) || (d3 > 0);

			return !(has_neg && has_pos);
		}

	}
}