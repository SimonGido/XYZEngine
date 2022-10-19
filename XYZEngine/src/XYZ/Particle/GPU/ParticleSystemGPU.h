#pragma once
#include "XYZ/Core/Timestep.h"

#include <glm/glm.hpp>

namespace XYZ {
	
	

	struct ParticleGPU
	{
		// Data that are rendered
		glm::vec4  TransformRow0;
		glm::vec4  TransformRow1;
		glm::vec4  TransformRow2;
		glm::vec4  RenderColor;

		// State of particle
		glm::vec4  Color;
		glm::vec4  Position;
		glm::vec4  Rotation;
		glm::vec4  Scale;
		glm::vec4  Velocity;

		float	   LifeRemaining;
		bool	   Alive = false;
		Padding<3> Padding0; // Bools are 4-bytes in GLSL
		Padding<8> Padding1;
		
	};

	struct ParticlePropertyGPU
	{
		// Spawn state
		glm::vec4 StartPosition;
		glm::vec4 StartColor;
		glm::vec4 StartRotation;
		glm::vec4 StartScale;
		glm::vec4 StartVelocity;
	
		// If module enabled, end state
		glm::vec4 EndColor;
		glm::vec4 EndRotation;
		glm::vec4 EndScale;
		glm::vec4 EndVelocity;


		float	  LifeTime;
	private:
		Padding<12> Padding;
	};

	class ParticleSystemGPU : public RefCount
	{
	public:
		ParticleSystemGPU();

		void Update(Timestep ts);


		std::vector<ParticlePropertyGPU> ParticleProperties;

		float	   Time = 0.0f;
		float	   Speed = 1.0f;
		uint32_t   MaxParticles;
		uint32_t   ParticlesEmitted = 0;
		int		   Loop = 1;
	};
}