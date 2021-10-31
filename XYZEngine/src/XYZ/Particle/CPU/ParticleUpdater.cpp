#include "stdafx.h"
#include "ParticleUpdater.h"

#include "XYZ/Scene/Components.h"
#include "XYZ/Renderer/SceneRenderer.h"

namespace XYZ {

	static float CalcRatio(float length, float value)
	{
		return (length - value) / length;
	}

	MainUpdater::MainUpdater()
		:
		m_Enabled(true)
	{
	}

	void MainUpdater::UpdateParticles(float ts, ParticleDataBuffer* data) const
	{
		if (m_Enabled)
		{
			uint32_t aliveParticles = data->GetAliveParticles();
			for (uint32_t i = 0; i < aliveParticles; ++i)
			{
				data->m_Particle[i].Position += data->m_Particle[i].Velocity * ts;
				data->m_Particle[i].LifeRemaining -= ts;
				if (data->m_Particle[i].LifeRemaining <= 0.0f)
				{
					data->Kill(i);
					aliveParticles--;
				}
			}
		}
	}

	LightUpdater::LightUpdater()
		:
		m_MaxLights(1000),
		m_Enabled(true)
	{
		
	}
	void LightUpdater::UpdateParticles(float ts, ParticleDataBuffer* data) const
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
	TextureAnimationUpdater::TextureAnimationUpdater()
		:
		m_Tiles(1, 1),
		m_StartFrame(0),
		m_CycleLength(5.0f),
		m_Enabled(true)
	{
	}
	void TextureAnimationUpdater::UpdateParticles(float ts, ParticleDataBuffer* data) const
	{
		if (m_Enabled)
		{
			uint32_t stageCount = m_Tiles.x * m_Tiles.y;
			float columnSize	= 1.0f / m_Tiles.x;
			float rowSize		= 1.0f / m_Tiles.y;
			
			uint32_t aliveParticles = data->GetAliveParticles();
			for (uint32_t i = 0; i < aliveParticles; ++i)
			{
				float ratio			= CalcRatio(m_CycleLength, data->m_Particle[i].LifeRemaining);
				float stageProgress = ratio * stageCount;
				
				uint32_t index  = (uint32_t)floor(stageProgress);
				float column	= index % m_Tiles.x;
				float row		= index / m_Tiles.y;
				
				data->m_TexOffset[i] = glm::vec2(column / m_Tiles.x, row / m_Tiles.y);
			}
		}
	}
	RotationOverLife::RotationOverLife()
		:
		m_EulerAngles(0.0f),
		m_CycleLength(5.0f),
		m_Enabled(true)
	{
	}
	void RotationOverLife::UpdateParticles(float ts, ParticleDataBuffer* data) const
	{
		if (m_Enabled)
		{
			uint32_t aliveParticles = data->GetAliveParticles();
			glm::vec3 radians = glm::radians(m_EulerAngles);
			for (uint32_t i = 0; i < aliveParticles; ++i)
			{
				float ratio = CalcRatio(m_CycleLength, data->m_Particle[i].LifeRemaining);
				data->m_Rotation[i] = glm::quat(radians * ratio);
			}
		}
	}
}