#include "stdafx.h"
#include "ParticleSystemCPUTest.h"

namespace XYZ {

	ParticleSystemCPUTest::ParticleSystemCPUTest(uint32_t maxParticles)
		:
		m_Particles(maxParticles),
		m_Renderer(maxParticles)
	{
		m_RenderData.resize(maxParticles);
	}

	void ParticleSystemCPUTest::Update(Timestep ts)
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
			m_Particles.m_Position[i] += m_Particles.m_Velocity[i] * ts.GetSeconds();
			m_RenderData[i] = ParticleRenderData{
				glm::vec4(1.0f, 1.0f, 1.0f, 1.0f),
				glm::vec4(0.0f, 0.0f, 1.0f, 1.0f),
				glm::vec2(m_Particles.m_Position[i].x, m_Particles.m_Position[i].y),
				glm::vec2(0.5f, 0.5f),
				m_Particles.m_Rotation[i]
			};
		}
		m_Renderer.InstanceCount = endId;
		m_Renderer.InstanceVBO->Update(m_RenderData.data(), endId * sizeof(ParticleRenderData));
	}

}