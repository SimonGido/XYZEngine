#include "stdafx.h"
#include "Math.h"


namespace XYZ {
	namespace Math {
		std::pair<glm::vec2, glm::vec2> CalculateVelocitiesAfterContact(const glm::vec2& firstVelocity, const glm::vec2& secondVelocity, float firstMass, float secondMass)
		{
			glm::vec2 firstMomentum = firstMass * firstVelocity;
			glm::vec2 secondMomentum = secondMass * secondVelocity;

			glm::vec2 momentum = firstMomentum + secondMomentum;
			glm::vec2 totalVelocity = firstVelocity + secondVelocity;

			glm::vec2 secondFinal = (firstMass * totalVelocity) / (firstMass + secondMass);
			glm::vec2 firstFinal = secondFinal - totalVelocity;

			return std::pair<glm::vec2, glm::vec2>(secondFinal, firstFinal);
		}
		glm::vec2 CalculateMirrorVector(const glm::vec2& dir, const glm::vec2& hitNormal)
		{
			return 2 * (glm::dot(dir, hitNormal)) * hitNormal - dir;
		}
	}
}