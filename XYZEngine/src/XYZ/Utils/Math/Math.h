#pragma once

#include <glm/glm.hpp>


namespace XYZ {

	namespace Math {

		std::pair<glm::vec2, glm::vec2> CalculateVelocitiesAfterContact(
			const glm::vec2& firstVelocity, const glm::vec2& secondVelocity, float firstMass, float secondMass
		);

		glm::vec2 CalculateMirrorVector(const glm::vec2& dir, const glm::vec2& hitNormal);
	}
}