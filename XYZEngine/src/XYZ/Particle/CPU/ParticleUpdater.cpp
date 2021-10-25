#include "stdafx.h"
#include "ParticleUpdater.h"

#include "XYZ/Scene/Components.h"
#include "XYZ/Renderer/SceneRenderer.h"

namespace XYZ {
	TimeUpdater::TimeUpdater()
		:
		m_Enabled(true)
	{
	}
	void TimeUpdater::UpdateParticles(float timeStep, ParticleDataBuffer* data) const
	{
		if (m_Enabled)
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


	PositionUpdater::PositionUpdater()
		:
		m_Enabled(true)
	{
	}

	void PositionUpdater::UpdateParticles(float timeStep, ParticleDataBuffer* data) const
	{
		if (m_Enabled)
		{
			uint32_t aliveParticles = data->GetAliveParticles();
			for (uint32_t i = 0; i < aliveParticles; ++i)
			{
				data->m_Particle[i].Position += data->m_Particle[i].Velocity * timeStep;
			}
		}
	}


	LightUpdater::LightUpdater()
		:
		m_MaxLights(1000),
		m_Enabled(true)
	{
		
	}
	void LightUpdater::UpdateParticles(float timeStep, ParticleDataBuffer* data) const
	{
		if (m_Enabled)
		{
			uint32_t aliveParticles = data->GetAliveParticles();

			if (m_TransformEntity.IsValid()
				&& m_LightEntity.IsValid()
				&& m_LightEntity.HasComponent<PointLight2D>())
			{
				for (uint32_t i = 0; i < aliveParticles && i < m_MaxLights; ++i)
				{
					data->m_Lights[i] = data->m_Particle[i].Position;
				}
			}
		}
	}

}