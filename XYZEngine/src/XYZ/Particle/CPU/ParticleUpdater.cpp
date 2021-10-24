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

	void TimeUpdater::SetEnable(bool enable)
	{
		m_Enabled = enable;
	}

	bool TimeUpdater::IsEnabled() const
	{
		return m_Enabled;
	}

	PositionUpdater::PositionUpdater()
		:
		m_Enabled(true)
	{
	}

	void PositionUpdater::UpdateParticles(float timeStep, ParticleDataBuffer* data) const
	{
		uint32_t aliveParticles = data->GetAliveParticles();
		for (uint32_t i = 0; i < aliveParticles; ++i)
		{
			data->m_Particle[i].Position += data->m_Particle[i].Velocity * timeStep;
		}
	}

	void PositionUpdater::SetEnable(bool enable)
	{
		m_Enabled = enable;
	}

	bool PositionUpdater::IsEnabled() const
	{
		return m_Enabled;
	}

	LightUpdater::LightUpdater()
		:
		m_MaxLights(50),
		m_Enabled(true)
	{
		
	}
	void LightUpdater::UpdateParticles(float timeStep, ParticleDataBuffer* data) const
	{
		uint32_t aliveParticles = data->GetAliveParticles();

		for (uint32_t i = 0; i < aliveParticles && i < m_MaxLights; ++i)
		{
			data->m_Lights[i] = data->m_Particle[i].Position;
		}
	}
	void LightUpdater::Update(Ref<SceneRenderer> renderer) const
	{
		if (m_LightEntity && m_LightEntity.HasComponent<PointLight2D>() && m_TransformEntity)
		{
			const PointLight2D& light = m_LightEntity.GetComponent<PointLight2D>();
			const TransformComponent& transform = m_TransformEntity.GetComponent<TransformComponent>();

			//auto lightRef = m_LightBuffer.ReadRead();
			//for (uint32_t i = 0; i < lightRef->LightCount; ++i)
			//{
			//	renderer->SubmitLight(light, transform.WorldTransform * glm::translate(lightRef->LightPositions[i]));
			//}
		}
	}
	void LightUpdater::SetEnable(bool enable)
	{
		m_Enabled = enable;
	}
	void LightUpdater::SetMaxLights(uint32_t maxLights)
	{
		m_MaxLights = maxLights;
	}
	void LightUpdater::SetLightEntity(const SceneEntity& entity)
	{
		m_LightEntity = entity;
	}
	void LightUpdater::SetTransformEntity(const SceneEntity& entity)
	{
		m_TransformEntity = entity;
	}

	uint32_t LightUpdater::GetMaxLights() const
	{
		return m_MaxLights;
	}

	SceneEntity LightUpdater::GetLightEntity() const
	{
		return m_LightEntity;
	}

	SceneEntity LightUpdater::GetTransformEntity() const
	{
		return m_TransformEntity;
	}

	bool LightUpdater::IsEnabled() const
	{
		return m_Enabled;
	}

}