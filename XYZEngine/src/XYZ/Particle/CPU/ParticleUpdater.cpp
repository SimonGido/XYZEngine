#include "stdafx.h"
#include "ParticleUpdater.h"


namespace XYZ {
	void BasicTimerUpdater::Update(float timeStep, ParticleDataBuffer* data)
	{
		uint32_t aliveParticles = data->GetAliveParticles();
		for (uint32_t i = 0; i < aliveParticles; ++i)
		{
			data->m_Particle[i].LifeRemaining -= timeStep;
			if (data->m_Particle[i].LifeRemaining <= 0.0f)
			{
				data->Kill(i);
				aliveParticles--;
			}
		}
	}
}