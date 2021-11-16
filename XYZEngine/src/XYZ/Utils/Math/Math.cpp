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
	}
}