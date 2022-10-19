#include "stdafx.h"
#include "ParticleSystemGPU.h"

#include <glm/common.hpp>
#include <glm/gtc/random.hpp>
#include <glm/gtc/constants.hpp>

namespace XYZ {
	ParticleSystemGPU::ParticleSystemGPU()
	{
		MaxParticles = 1024;
		ParticlesEmitted = MaxParticles;
		ParticleProperties.resize(MaxParticles);

		Time = 0.0f;

		glm::vec4 MinVelocity(-0.5f);
		glm::vec4 MaxVelocity(0.5f);
		for (size_t i = 0; i < ParticleProperties.size(); ++i)
		{
			float startScale = glm::linearRand(0.1f, 2.0f);
			float endScale = glm::linearRand(0.1f, 2.0f);

			ParticleProperties[i].StartPosition = glm::vec4(0.0f);
			ParticleProperties[i].StartColor	= glm::linearRand(glm::vec4(0.5f, 0.5f, 0.5f, 1.0f), glm::vec4(1.0f));
			ParticleProperties[i].StartRotation = glm::linearRand(glm::vec4(-90.0f), glm::vec4(90.0f));
			ParticleProperties[i].StartScale	= glm::vec4(startScale);
			ParticleProperties[i].StartVelocity = glm::linearRand(MinVelocity, MaxVelocity);
		
			ParticleProperties[i].EndColor		= glm::linearRand(glm::vec4(0.8f, 0.4f, 0.5f, 1.0f), glm::vec4(1.0f));
			ParticleProperties[i].EndRotation	= glm::linearRand(glm::vec4(-90.0f), glm::vec4(90.0f));
			ParticleProperties[i].EndScale		= glm::vec4(endScale);
			ParticleProperties[i].EndVelocity	= glm::linearRand(MinVelocity, MaxVelocity);

			ParticleProperties[i].LifeTime = glm::linearRand(1.0f, 5.0f);
		}
	}
	void ParticleSystemGPU::Update(Timestep ts)
	{
	}
}