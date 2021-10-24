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
			m_LightBuffer.Read()->LightPositions.resize(50);
		}
		{
			m_LightBuffer.Write()->LightPositions.resize(50);
		}
	}
	void LightUpdater::UpdateParticles(float timeStep, ParticleDataBuffer* data)
	{
		{
			std::scoped_lock lock(m_Mutex);
			uint32_t aliveParticles = data->GetAliveParticles();

			auto val = m_LightBuffer.Write();
			val->LightCount = 0;
			for (uint32_t i = 0; i < aliveParticles && i < m_MaxLights; ++i)
			{
				val->LightPositions[i] = data->m_Particle[i].Position;
				val->LightCount++;
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
			for (uint32_t i = 0; i < lightRef->LightCount; ++i)
			{
				XYZ_WARN("Particle light updater is currently not working");
				//SceneRenderer::SubmitLight(*light, transform.WorldTransform * glm::translate(lightRef->LightPositions[i]));
			}
		}
	}
	void LightUpdater::SetMaxLights(uint32_t maxLights)
	{
		std::scoped_lock lock(m_Mutex);
		m_MaxLights = maxLights;
		{
			auto val = m_LightBuffer.Read();
			val->LightPositions.resize(m_MaxLights);
			val->LightCount = std::min((uint32_t)val->LightPositions.size(), val->LightCount);
		}
		{
			auto val = m_LightBuffer.Write();
			val->LightPositions.resize(m_MaxLights);
			val->LightCount = std::min((uint32_t)val->LightPositions.size(), val->LightCount);
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