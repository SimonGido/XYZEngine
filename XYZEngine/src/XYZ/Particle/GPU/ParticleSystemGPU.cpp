#include "stdafx.h"
#include "ParticleSystemGPU.h"

#include <glm/common.hpp>
#include <glm/gtc/random.hpp>
#include <glm/gtc/constants.hpp>

namespace XYZ {
	ParticleSystemGPU::ParticleSystemGPU()
	{
		Particles.resize(1024);
		ParticleProperties.resize(1024);

		MaxParticles = 1024;

		LifeTime = 5.0f;
		Time = 0.0f;
	
		glm::vec4 MinVelocity(-0.5f);
		glm::vec4 MaxVelocity(0.5f);
		for (size_t i = 0; i < ParticleProperties.size(); ++i)
		{
			ParticleProperties[i].StartPosition = glm::vec4(0.0f);
			ParticleProperties[i].StartVelocity = glm::linearRand(MinVelocity, MaxVelocity);;
			ParticleProperties[i].StartColor	= glm::vec4(1.0f);
		}
	}
}