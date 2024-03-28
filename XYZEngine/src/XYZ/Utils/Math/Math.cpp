#include "stdafx.h"
#include "Math.h"

#include <glm/gtx/transform.hpp>
#include <glm/gtx/matrix_decompose.hpp>

namespace XYZ {
	namespace Math {
		std::pair<glm::vec2, glm::vec2> CalculateVelocitiesAfterContact(const glm::vec2& firstVelocity, const glm::vec2& secondVelocity, float firstMass, float secondMass)
		{
			const glm::vec2 firstMomentum = firstMass * firstVelocity;
			const glm::vec2 secondMomentum = secondMass * secondVelocity;

			glm::vec2 momentum = firstMomentum + secondMomentum;
			const glm::vec2 totalVelocity = firstVelocity + secondVelocity;

			glm::vec2 secondFinal = (firstMass * totalVelocity) / (firstMass + secondMass);
			glm::vec2 firstFinal = secondFinal - totalVelocity;

			return std::pair<glm::vec2, glm::vec2>(secondFinal, firstFinal);
		}
		glm::vec2 CalculateMirrorVector(const glm::vec2& dir, const glm::vec2& hitNormal)
		{
			return 2 * (glm::dot(dir, hitNormal)) * hitNormal - dir;
		}
		float PythagoreanSolve(float a, float b)
		{
			return sqrt(a * a + b * b);
		}
		void DecomposeTransform(const glm::mat4& transform, glm::vec3& translation, glm::vec3& rotation, glm::vec3& scale)
		{
			glm::quat rotq;
			glm::vec3 skew;
			glm::vec4 perspective;

			glm::decompose(transform, scale, rotq, translation, skew, perspective);
			rotation = glm::eulerAngles(rotq);
		}
		glm::vec3 TransformToTranslation(const glm::mat4& transform)
		{
			glm::vec3 translation, scale;
			glm::quat rotq;
			glm::vec3 skew;
			glm::vec4 perspective;

			glm::decompose(transform, scale, rotq, translation, skew, perspective);
			return translation;
		}
		XYZ_API bool PointInBox(const glm::vec3& point, const glm::vec3& boxMin, const glm::vec3& boxMax)
		{
			return
				point.x >= boxMin.x
				&& point.x <= boxMax.x
				&& point.y >= boxMin.y
				&& point.y <= boxMax.y
				&& point.z >= boxMin.z
				&& point.z <= boxMax.z;
		}
		std::tuple<glm::vec3, glm::vec3, glm::vec3> DecomposeTransform(const glm::mat4& transform)
		{
			glm::vec3 translation;
			glm::vec3 scale;
			glm::quat rot;
			glm::vec3 skew;
			glm::vec4 perspective;
			glm::decompose(transform, scale, rot, translation, skew, perspective);
			glm::vec3 euler = glm::eulerAngles(rot);
			return std::tuple<glm::vec3, glm::vec3, glm::vec3>(translation, euler, scale);
		}

		std::tuple<glm::vec3, glm::quat, glm::vec3> DecomposeTransformQuat(const glm::mat4& transform)
		{
			glm::vec3 translation;
			glm::vec3 scale;
			glm::quat rot;
			glm::vec3 skew;
			glm::vec4 perspective;
			glm::decompose(transform, scale, rot, translation, skew, perspective);
			return std::tuple<glm::vec3, glm::quat, glm::vec3>(translation, rot, scale);
		}

		int32_t RoundUp(int32_t numToRound, int32_t multiple)
		{
			int32_t isPositive = (int32_t)(numToRound >= 0);
			return ((numToRound + isPositive * (multiple - 1)) / multiple) * multiple;
		}
	}
}