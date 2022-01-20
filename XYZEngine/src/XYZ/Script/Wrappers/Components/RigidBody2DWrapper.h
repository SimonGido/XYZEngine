#pragma once

#include <glm/glm.hpp>

namespace XYZ {
	namespace Script {
		struct RigidBody2DNative
		{
			static void Register();
			static void ApplyForce(uint32_t entity, glm::vec2* impulse, glm::vec2* point);
		};
	}
}