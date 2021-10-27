#include "stdafx.h"
#include "ParticleEmitter.h"


namespace XYZ {
	ParticleEmitterCPU::ParticleEmitterCPU()
		:
		m_EmitRate(0.0f),
		m_EmittedParticles(10.0f)
	{
	}
	void ParticleEmitterCPU::Emit(Timestep ts, ParticleDataBuffer& particles)
	{
		m_EmittedParticles += m_EmitRate * ts;

		const uint32_t newParticles = (uint32_t)m_EmittedParticles + m_BurstEmitter.Update(ts);
		const uint32_t startId = particles.GetAliveParticles();
		const uint32_t endId = std::min(startId + newParticles, particles.GetMaxParticles() - 1);
		if (newParticles)
			m_EmittedParticles = 0.0f;

		m_ShapeGenerator.Generate(&particles, startId, endId);
		m_LifeGenerator.Generate(&particles, startId, endId);
		m_RandomVelGenerator.Generate(&particles, startId, endId);

		for (uint32_t i = startId; i < endId; ++i)
			particles.Wake(i);
	}

	BurstEmitter::BurstEmitter()
		:
		m_Interval(5.0f),
		m_PassedTime(0.0f)
	{
	}
	uint32_t BurstEmitter::Update(Timestep ts)
	{
		uint32_t count = 0;
		for (auto& burst : m_Bursts)
		{
			if (!burst.m_Called && m_PassedTime >= burst.m_Time)
			{
				count += burst.m_Count;
				burst.m_Called = true;
				break;
			}
		}
		m_PassedTime += ts.GetSeconds();
		if (m_PassedTime >= m_Interval)
		{
			m_PassedTime = 0.0f;
			reset();
		}
		return count;
	}
	void BurstEmitter::reset()
	{
		for (auto& burst : m_Bursts)
			burst.m_Called = false;
	}
	BurstEmitter::Burst::Burst()
		:
		m_Count(0),
		m_Time(0.0f),
		m_Called(false)
	{
	}
	BurstEmitter::Burst::Burst(uint32_t count, float time)
		:
		m_Count(count),
		m_Time(time),
		m_Called(false)
	{
	}
}