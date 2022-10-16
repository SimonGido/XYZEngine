#pragma once

#include <glm/glm.hpp>

namespace XYZ {
	
	

	struct ParticleGPU
	{
		glm::vec4    TransformRow0;
		glm::vec4    TransformRow1;
		glm::vec4    TransformRow2;
		glm::vec4    Color;
		
	};

	struct ParticlePropertyGPU
	{
		glm::vec4 StartPosition;
		glm::vec4 StartVelocity;
		glm::vec4 StartColor;

		glm::vec4 Position;
		glm::vec4 Color;
		glm::vec4 Velocity;

		float	   LifeRemaining;
		bool	   Alive = false;
		Padding<3> Padding0; // Bools are 4-bytes in GLSL
		Padding<8> Padding1;
	};

	class ParticleSystemGPU : public RefCount
	{
	public:
		ParticleSystemGPU();

		std::vector<ParticleGPU> Particles;
		std::vector<ParticlePropertyGPU> ParticleProperties;

		glm::vec4  EndColor;
		glm::vec3  EndRotation;
		glm::vec3  EndSize;
		float	   LifeTime;
		float	   Time = 0.0f;
		float	   Speed = 1.0f;
		uint32_t   MaxParticles;
		uint32_t   ParticlesEmitted = 0;
		int		   Loop = 1;
	};
}