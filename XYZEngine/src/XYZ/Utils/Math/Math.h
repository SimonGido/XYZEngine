#pragma once

#include <glm/glm.hpp>


namespace XYZ {

	namespace Math {

		std::pair<glm::vec2, glm::vec2> CalculateVelocitiesAfterContact(
			const glm::vec2& firstVelocity, const glm::vec2& secondVelocity, float firstMass, float secondMass
		);

		glm::vec2 CalculateMirrorVector(const glm::vec2& dir, const glm::vec2& hitNormal);

		float PythagoreanSolve(float a, float b);


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
	}
}