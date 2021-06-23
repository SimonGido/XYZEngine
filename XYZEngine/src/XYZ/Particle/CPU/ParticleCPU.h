#pragma once

#include <glm/glm.hpp>

namespace XYZ {

	struct ParticleCPU
	{
		glm::vec4 Color;
		glm::vec4 TexCoord;
		glm::vec3 Position;
		glm::vec3 Velocity;
		glm::vec2 Size;

		float Rotation;
		float AngularVelocity;
		float LifeRemaining;
		bool  Alive;
	};

}