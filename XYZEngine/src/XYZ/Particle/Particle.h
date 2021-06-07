#pragma once

#include <glm/glm.hpp>

namespace XYZ {

	struct ParticleVertex
	{
		glm::vec3 Position;
		glm::vec2 TexCoord;
	};

	struct ParticleData
	{
		glm::vec4 Color			 = glm::vec4(0);
		glm::vec4 Position		 = glm::vec4(0);
		glm::vec2 TexCoord		 = glm::vec2(0);
		glm::vec2 Size			 = glm::vec2(0);
		float	  Rotation		 = 0.0f;

	private:
		float	  Alignment[3];
	};

	struct ParticleSpecification
	{
		glm::vec4 StartColor;	
		glm::vec2 StartSize;
		glm::vec2 StartVelocity;
		glm::vec2 StartPosition;

		float	  LifeTime;
	private:
		float	  TimeAlive;
		int32_t   IsAlive;
		
		float	  Allignment[3];
	};
}