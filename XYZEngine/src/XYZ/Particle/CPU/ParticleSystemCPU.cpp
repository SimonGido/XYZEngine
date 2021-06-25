#include "stdafx.h"
#include "ParticleSystemCPU.h"


namespace XYZ {

	ParticleSystemCPU::ParticleSystemCPU(uint32_t maxParticles)
		:
		m_Particles(maxParticles),
		m_Renderer(maxParticles)
	{
		m_RenderData.resize(maxParticles);
	}

	void ParticleSystemCPU::Update(Timestep ts)
	{
		for (auto generator : m_Generators)
		{
			uint32_t startId = m_Particles.GetAliveParticles();
			generator->Generate(&m_Particles, startId, ts.GetSeconds());
		}
		for (auto updater : m_Updaters)
		{
			updater->Update(ts.GetSeconds(), &m_Particles);
		}
		uint32_t endId = m_Particles.GetAliveParticles();
	
		for (uint32_t i = 0; i < endId; ++i)
		{
			m_Particles.m_Particle[i].Position += m_Particles.m_Particle[i].Velocity * ts.GetSeconds();
			m_RenderData[i] = ParticleRenderData{
				glm::vec4(1.0f, 1.0f, 1.0f, 1.0f),
				glm::vec4(0.0f, 0.0f, 1.0f, 1.0f),
				glm::vec2(m_Particles.m_Particle[i].Position.x, m_Particles.m_Particle[i].Position.y),
				glm::vec2(0.5f, 0.5f),
				m_Particles.m_Rotation[i]
			};
		}
		m_Renderer.InstanceCount = endId;
		m_Renderer.InstanceVBO->Update(m_RenderData.data(), endId * sizeof(ParticleRenderData));
	}

}