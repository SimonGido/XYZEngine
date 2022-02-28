#include "stdafx.h"
#include "ParticleUpdater.h"

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


	LightUpdater::LightUpdater()
		:
		MaxLights(20)
	{		
	}

	void LightUpdater::UpdateParticles(float ts, ParticlePool& data)
	{
		if (Enabled)
		{
			Lights.clear();
			const uint32_t aliveParticles = data.GetAliveParticles();
			
			for (uint32_t i = 0; i < aliveParticles && i < MaxLights; ++i)
			{
				Lights.push_back(data.Particles[i].Position);
			}
		}
	}

	void LightUpdater::Reset()
	{

	}

	TextureAnimationUpdater::TextureAnimationUpdater()
		:
		Tiles(1, 1),
		StartFrame(0),
		CycleLength(5.0f)
	{
	}
	void TextureAnimationUpdater::UpdateParticles(float ts, ParticlePool& data) const
	{
		if (Enabled)
		{
			const uint32_t stageCount =  Tiles.x * Tiles.y;
			float columnSize	= 1.0f / Tiles.x;
			float rowSize		= 1.0f / Tiles.y;

			const uint32_t aliveParticles = data.GetAliveParticles();
			for (uint32_t i = 0; i < aliveParticles; ++i)
			{
				const float ratio			= CalcRatio(CycleLength, data.Particles[i].LifeRemaining);
				const float stageProgress = ratio * stageCount;

				const uint32_t index  = (uint32_t)floor(stageProgress);
				const float column	= index % Tiles.x;
				const float row		= index / Tiles.y;
				
				data.Particles[i].TexOffset = glm::vec2(column / Tiles.x, row / Tiles.y);
			}
		}
	}

	RotationOverLife::RotationOverLife()
		:
		EulerAngles(0.0f),
		CycleLength(5.0f)
	{
	}
	void RotationOverLife::UpdateParticles(float ts, ParticlePool& data) const
	{
		if (Enabled)
		{
			const uint32_t aliveParticles = data.GetAliveParticles();
			const glm::vec3 radians = glm::radians(EulerAngles);
			for (uint32_t i = 0; i < aliveParticles; ++i)
			{
				const float ratio = CalcRatio(CycleLength, data.Particles[i].LifeRemaining);
				data.Particles[i].Rotation = glm::quat(radians * ratio);
			}
		}
	}
}