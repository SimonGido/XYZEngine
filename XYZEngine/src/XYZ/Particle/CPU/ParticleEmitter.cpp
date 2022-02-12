#include "stdafx.h"
#include "ParticleEmitter.h"


namespace XYZ {
	ParticleEmitterCPU::ParticleEmitterCPU()
		:
		EmitRate(5.0f),
		m_EmittedParticles(0.0f)
	{
	}
	void ParticleEmitterCPU::Emit(Timestep ts, ParticleDataBuffer& particles)
	{
		m_EmittedParticles += EmitRate * ts;

		const uint32_t newParticles = (uint32_t)m_EmittedParticles + BurstEmitter.Update(ts);
		const uint32_t startId = particles.GetAliveParticles();
		const uint32_t endId = std::min(startId + newParticles, particles.GetMaxParticles() - 1);
		if (newParticles)
			m_EmittedParticles = 0.0f;

		
		ShapeGenerator.Generate(particles, startId, endId);
		LifeGenerator.Generate(particles, startId, endId);
		RandomVelGenerator.Generate(particles, startId, endId);
		for (uint32_t i = startId; i < endId; ++i)
			particles.Wake(i);
	}

	BurstEmitter::BurstEmitter()
		:
		Interval(5.0f),
		m_PassedTime(0.0f)
	{
	}
	uint32_t BurstEmitter::Update(Timestep ts)
	{
		uint32_t count = 0;
		for (auto& burst : Bursts)
		{
			if (!burst.m_Called && m_PassedTime >= burst.Time)
			{
				count += burst.Count;
				burst.m_Called = true;
				break;
			}
		}
		m_PassedTime += ts.GetSeconds();
		if (m_PassedTime >= Interval)
		{
			m_PassedTime = 0.0f;
			reset();
		}
		return count;
	}
	void BurstEmitter::reset()
	{
		for (auto& burst : Bursts)
			burst.m_Called = false;
	}
	BurstEmitter::Burst::Burst()
		:
		Count(0),
		Time(0.0f),
		m_Called(false)
	{
	}
	BurstEmitter::Burst::Burst(uint32_t count, float time)
		:
		Count(count),
		Time(time),
		m_Called(false)
	{
	}
}