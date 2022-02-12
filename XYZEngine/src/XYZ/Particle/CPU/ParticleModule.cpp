#include "stdafx.h"
#include "ParticleModule.h"

#include "XYZ/Scene/Components.h"
#include "XYZ/Renderer/SceneRenderer.h"

#include "XYZ/Utils/Math/Math.h"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>

#include <glm/gtx/rotate_vector.hpp>

namespace XYZ {

	static float CalcRatio(float length, float value)
	{
		return (length - value) / length;
	}

	MainModule::MainModule()
	{
	}

	void MainModule::UpdateParticles(float ts, ParticleDataBuffer& data)
	{
		if (IsEnabled())
		{
			m_Killed.clear();
			uint32_t aliveParticles = data.GetAliveParticles();
			for (uint32_t i = 0; i < aliveParticles; ++i)
			{
				data.Particle[i].Position += data.Particle[i].Velocity * ts;
				data.Particle[i].LifeRemaining -= ts;
				if (data.Particle[i].LifeRemaining <= 0.0f)
				{		
					aliveParticles--;
					m_Killed.push_back(i);
				}
			}
		}
	}



	LightModule::LightModule()
		:
		MaxLights(1000)
	{
		
	}
	void LightModule::UpdateParticles(float ts, ParticleDataBuffer& data)
	{
		if (IsEnabled())
		{
			m_Lights.clear();
			const uint32_t aliveParticles = data.GetAliveParticles();
			//if (m_TransformEntity.IsValid()
			//	&& m_LightEntity.IsValid()
			//	&& m_LightEntity.HasComponent<PointLight2D>())
			//{
			//	for (uint32_t i = 0; i < aliveParticles && i < m_MaxLights; ++i)
			//	{
			//		m_Lights.push_back(data.m_Particle[i].Position);
			//	}
			//}
		}
	}

	void LightModule::Reset()
	{

	}

	TextureAnimationModule::TextureAnimationModule()
		:
		Tiles(1, 1),
		StartFrame(0),
		CycleLength(5.0f)
	{
	}
	void TextureAnimationModule::UpdateParticles(float ts, ParticleDataBuffer& data) const
	{
		if (IsEnabled())
		{
			const uint32_t stageCount =  Tiles.x * Tiles.y;
			float columnSize	= 1.0f / Tiles.x;
			float rowSize		= 1.0f / Tiles.y;

			const uint32_t aliveParticles = data.GetAliveParticles();
			for (uint32_t i = 0; i < aliveParticles; ++i)
			{
				const float ratio			= CalcRatio(CycleLength, data.Particle[i].LifeRemaining);
				const float stageProgress = ratio * stageCount;

				const uint32_t index  = (uint32_t)floor(stageProgress);
				const float column	= index % Tiles.x;
				const float row		= index / Tiles.y;
				
				data.TexOffset[i] = glm::vec2(column / Tiles.x, row / Tiles.y);
			}
		}
	}

	RotationOverLife::RotationOverLife()
		:
		EulerAngles(0.0f),
		CycleLength(5.0f)
	{
	}
	void RotationOverLife::UpdateParticles(float ts, ParticleDataBuffer& data) const
	{
		if (IsEnabled())
		{
			const uint32_t aliveParticles = data.GetAliveParticles();
			const glm::vec3 radians = glm::radians(EulerAngles);
			for (uint32_t i = 0; i < aliveParticles; ++i)
			{
				const float ratio = CalcRatio(CycleLength, data.Particle[i].LifeRemaining);
				data.Rotation[i] = glm::quat(radians * ratio);
			}
		}
	}
}