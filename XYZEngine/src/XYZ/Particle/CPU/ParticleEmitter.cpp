#include "stdafx.h"
#include "ParticleEmitter.h"

#include "XYZ/Utils/Random.h"

namespace XYZ {
	ParticleEmitter::ParticleEmitter()
		:
		EmitRate(5.0f),
		BurstInterval(5.0f),
		m_EmittedParticles(0.0f),
		m_PassedTime(0.0f)
	{
	}
	std::pair<uint32_t, uint32_t> ParticleEmitter::Emit(Timestep ts, ParticleDataBuffer& particles)
	{		
		m_EmittedParticles += EmitRate * ts;

		const uint32_t newParticles = (uint32_t)m_EmittedParticles + burstEmit();
		const uint32_t startId = particles.GetAliveParticles();
		const uint32_t endId = std::min(startId + newParticles, particles.GetMaxParticles() - 1);
		if (newParticles)
			m_EmittedParticles = 0.0f;
		
		
		for (uint32_t i = startId; i < endId; ++i)
			particles.Wake(i);

		m_PassedTime += ts;

		return { startId, endId };
	}

	uint32_t ParticleEmitter::burstEmit()
	{
		uint32_t count = 0;
		for (auto& burst : Bursts)
		{
			if (!burst.m_Called && burst.Time <= m_PassedTime)
			{
				if (RandomNumber(0.0f, 1.0f) <= burst.Probability)
				{
					count += burst.Count;
					burst.m_Called = true;
				}
			}
		}

		if (m_PassedTime >= BurstInterval)
		{
			m_PassedTime = 0.0f;
			for (auto& burst : Bursts)
				burst.m_Called = false;
		}
		return count;
	}
	EmitterBurst::EmitterBurst(uint32_t count, float time, float probability)
		:
		Count(count), Time(time), Probability(probability)
	{
	}
}