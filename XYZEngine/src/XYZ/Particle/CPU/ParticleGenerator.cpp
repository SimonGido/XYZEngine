#include "stdafx.h"
#include "ParticleGenerator.h"


#include <glm/common.hpp>
#include <glm/gtc/random.hpp>

namespace XYZ {

	ParticleGenerator::ParticleGenerator()
		:
		EmittedParticles(0.0f),
		EmitRate(3.0f),
		LifeTime(2.0f)
	{
	}

	ParticleBoxGenerator::ParticleBoxGenerator()
		:
		BoxMin(-1.0f),
		BoxMax(1.0f)
	{
	}

	void ParticleBoxGenerator::Generate(ParticleDataBuffer* data, uint32_t startId, float timeStep)
	{
		EmittedParticles += timeStep * EmitRate;
		uint32_t endId = startId + (uint32_t)std::floor(EmittedParticles);
		if (endId > startId)
		{
			EmittedParticles = 0.0f;
			std::random_device dev;
			std::mt19937 rng(dev());
			std::uniform_real_distribution<double> dist(-2.0, 2.0); // distribution in range [1, 6]

			endId = std::min(endId, data->GetMaxParticles());
			for (size_t i = startId; i < endId; i++)
			{
				data->Wake(i);
				data->m_Particle[i].Color = glm::vec4(glm::linearRand(0.0f, 1.0f));
				data->m_Particle[i].Position = glm::vec4(glm::linearRand(BoxMin, BoxMax));
				data->m_Particle[i].Velocity = glm::vec3(dist(rng), dist(rng), 0.0f);
				data->m_Particle[i].LifeRemaining = LifeTime;
				data->m_TexCoord[i] = glm::vec4(0.5f, 0.5f, 0.75f, 0.75f);
				data->m_Size[i] = glm::vec2(0.5f);
			}
		}
	}
	
}