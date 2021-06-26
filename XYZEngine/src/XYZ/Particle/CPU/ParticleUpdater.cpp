#include "stdafx.h"
#include "ParticleUpdater.h"

#include "XYZ/Scene/Components.h"
#include "XYZ/Renderer/SceneRenderer.h"

namespace XYZ {
	ParticleUpdater::ParticleUpdater()
	{
	}

	void BasicTimerUpdater::UpdateParticles(float timeStep, ParticleDataBuffer* data)
	{
		std::scoped_lock lock(m_Mutex);

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

	void PositionUpdater::UpdateParticles(float timeStep, ParticleDataBuffer* data)
	{
		std::scoped_lock lock(m_Mutex);
		uint32_t aliveParticles = data->GetAliveParticles();
		for (uint32_t i = 0; i < aliveParticles; ++i)
		{
			data->m_Particle[i].Position += data->m_Particle[i].Velocity * timeStep;
		}
	}
	LightUpdater::LightUpdater()
		:
		m_MaxLights(50)
	{
		{
			m_LightBuffer.Read().Get().LightPositions.resize(50);
		}
		{
			m_LightBuffer.Write().Get().LightPositions.resize(50);
		}
	}
	void LightUpdater::UpdateParticles(float timeStep, ParticleDataBuffer* data)
	{
		{
			std::scoped_lock lock(m_Mutex);
			uint32_t aliveParticles = data->GetAliveParticles();

			auto val = m_LightBuffer.Write();
			val.Get().LightCount = 0;
			for (uint32_t i = 0; i < aliveParticles && i < m_MaxLights; ++i)
			{
				val.Get().LightPositions[i] = data->m_Particle[i].Position;
				val.Get().LightCount++;
			}
		}
		m_LightBuffer.AttemptSwap();
	}
	void LightUpdater::Update()
	{
		std::scoped_lock lock(m_Mutex);
		if (m_LightEntity && m_LightEntity.HasComponent<PointLight2D>() && m_TransformEntity)
		{
			PointLight2D* light = &m_LightEntity.GetComponent<PointLight2D>();
			TransformComponent& transform = m_TransformEntity.GetComponent<TransformComponent>();

			auto lightRef = m_LightBuffer.Read();
			for (uint32_t i = 0; i < lightRef.Get().LightCount; ++i)
			{
				SceneRenderer::SubmitLight(light, transform.WorldTransform * glm::translate(lightRef.Get().LightPositions[i]));
			}
		}
	}
	void LightUpdater::SetMaxLights(uint32_t maxLights)
	{
		std::scoped_lock lock(m_Mutex);
		m_MaxLights = maxLights;
		{
			auto val = m_LightBuffer.Read();
			val.Get().LightPositions.resize(m_MaxLights);
			val.Get().LightCount = std::min((uint32_t)val.Get().LightPositions.size(), val.Get().LightCount);
		}
		{
			auto val = m_LightBuffer.Write();
			val.Get().LightPositions.resize(m_MaxLights);
			val.Get().LightCount = std::min((uint32_t)val.Get().LightPositions.size(), val.Get().LightCount);
		}
	}
	void LightUpdater::SetLightEntity(SceneEntity entity)
	{
		std::scoped_lock lock(m_Mutex);
		m_LightEntity = entity;
	}
	void LightUpdater::SetTransformEntity(SceneEntity entity)
	{
		std::scoped_lock lock(m_Mutex);
		m_TransformEntity = entity;
	}

	uint32_t LightUpdater::GetMaxLights() const
	{
		std::scoped_lock lock(m_Mutex);
		return m_MaxLights;
	}

	SceneEntity LightUpdater::GetLightEntity() const
	{
		std::scoped_lock lock(m_Mutex);
		return m_LightEntity;
	}

	SceneEntity LightUpdater::GetTransformEntity() const
	{
		std::scoped_lock lock(m_Mutex);
		return m_TransformEntity;
	}

}