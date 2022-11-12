#pragma once
#include "XYZ/Core/Core.h"

#include <glm/glm.hpp>


namespace XYZ {

	namespace Math {

		XYZ_API std::pair<glm::vec2, glm::vec2> CalculateVelocitiesAfterContact(
			const glm::vec2& firstVelocity, const glm::vec2& secondVelocity, float firstMass, float secondMass
		);

		XYZ_API glm::vec2 CalculateMirrorVector(const glm::vec2& dir, const glm::vec2& hitNormal);

		XYZ_API float PythagoreanSolve(float a, float b);

		XYZ_API void DecomposeTransform(const glm::mat4& transform, glm::vec3& translation, glm::vec3& rotation, glm::vec3& scale);
	
		XYZ_API glm::vec3 TransformToTranslation(const glm::mat4& transform);

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

		XYZ_API std::tuple<glm::vec3, glm::vec3, glm::vec3> DecomposeTransform(const glm::mat4& transform);

		XYZ_API std::tuple<glm::vec3, glm::quat, glm::vec3> DecomposeTransformQuat(const glm::mat4& transform);
	
	
		XYZ_API int32_t RoundUp(int32_t numToRound, int32_t multiple);
		
	}
}