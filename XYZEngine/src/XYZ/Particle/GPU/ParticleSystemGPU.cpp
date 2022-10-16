#include "stdafx.h"
#include "ParticleSystemGPU.h"


namespace XYZ {
	ParticleSystemGPU::ParticleSystemGPU()
	{
		Particles.resize(1024);
		ParticleProperties.resize(1024);

		MaxParticles = 1024;

		LifeTime = 10.0f;
		Time = 0.0f;
	
		for (size_t i = 0; i < ParticleProperties.size(); ++i)
		{
			ParticleProperties[i].StartPosition = glm::vec4(0.0f);
			ParticleProperties[i].StartVelocity = glm::vec4(0.5f);
			ParticleProperties[i].StartColor	= glm::vec4(1.0f);
		}
	}
}