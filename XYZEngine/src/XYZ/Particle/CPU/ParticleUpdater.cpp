#include "stdafx.h"
#include "ParticleUpdater.h"

#include "XYZ/Scene/Components.h"
#include "XYZ/Renderer/SceneRenderer.h"

namespace XYZ {
	void BasicTimerUpdater::UpdateParticles(float timeStep, ParticleDataBuffer* data)
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
	void PositionUpdater::UpdateParticles(float timeStep, ParticleDataBuffer* data)
	{
		uint32_t aliveParticles = data->GetAliveParticles();
		for (uint32_t i = 0; i < aliveParticles; ++i)
		{
			data->m_Particle[i].Position += data->m_Particle[i].Velocity * timeStep;
		}
	}
	LightUpdater::LightUpdater()
		:
		MaxLights(10)
	{
		Light = new PointLight2D();
		{
			LightBuffer.Read().Get().LightPositions.resize(10);
		}
		{
			LightBuffer.Write().Get().LightPositions.resize(10);
		}
	}
	void LightUpdater::UpdateParticles(float timeStep, ParticleDataBuffer* data)
	{
		{
			uint32_t aliveParticles = data->GetAliveParticles();

			auto val = LightBuffer.Write();
			val.Get().LightCount = 0;
			for (uint32_t i = 0; i < aliveParticles && i < MaxLights; ++i)
			{
				val.Get().LightPositions[i] = data->m_Particle[i].Position;
				val.Get().LightCount++;
			}
		}
		LightBuffer.AttemptSwap();
	}
	void LightUpdater::Update()
	{
		auto lightRef = LightBuffer.Read();
		for (uint32_t i = 0; i < lightRef.Get().LightCount; ++i)
		{
			SceneRenderer::SubmitLight(Light,  glm::translate(lightRef.Get().LightPositions[i]));
		}
	}
	void LightUpdater::SetMaxLights(uint32_t maxLights)
	{
		MaxLights = maxLights;
		{
			LightBuffer.Read().Get().LightPositions.resize(MaxLights);
		}
		{
			LightBuffer.Write().Get().LightPositions.resize(MaxLights);
		}
	}
}