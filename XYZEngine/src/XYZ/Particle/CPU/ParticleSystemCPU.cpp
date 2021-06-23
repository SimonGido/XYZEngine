#include "stdafx.h"
#include "ParticleSystemCPU.h"


namespace XYZ {
	ParticleSystemCPU::ParticleSystemCPU(uint32_t maxParticles)
		:
		m_MaxParticles(maxParticles),
		m_ParticlesAlive(0),
		m_EmittedParticles(0.0f)
	{
	}
	ParticleSystemCPU::~ParticleSystemCPU()
	{
	}
	void ParticleSystemCPU::SetMaxParticles(uint32_t maxParticles)
	{
		m_MaxParticles = maxParticles;
	}
	void ParticleSystemCPU::Update(Timestep ts)
	{
		m_EmittedParticles += m_Emission.RateOverTime * ts;
		if (m_EmittedParticles > 1.0f)
		{
			emitt((uint32_t)m_EmittedParticles);
			m_EmittedParticles = 0.0f;
		}
		for (size_t i = 0; i < m_ParticlePool.size(); ++i)
		{
			auto& particle = m_ParticlePool[i];
			if (!particle.Alive)
				break;

			if (particle.LifeRemaining <= 0.0f)
			{
				// Copy last alive at the place of dead particle
				particle = m_ParticlePool[m_ParticlesAlive - 1];
				m_ParticlePool[m_ParticlesAlive - 1].Alive = false;
				m_ParticlesAlive--;
				continue;
			}

			particle.LifeRemaining -= ts;
			particle.Position += particle.Velocity * (float)ts;
			particle.Rotation += particle.AngularVelocity * ts;
		}
	}
	void ParticleSystemCPU::emitt(uint32_t count)
	{
		for (uint32_t i = 0; i < count && m_ParticlesAlive < m_MaxParticles; ++i)
		{
			ParticleCPU& particle = m_ParticlePool[m_ParticlesAlive];
			particle.Alive = true;
			particle.Position = glm::vec3(0.0f);
			particle.LifeRemaining = 3.0f;
			m_ParticlesAlive++;
		}
	}
}