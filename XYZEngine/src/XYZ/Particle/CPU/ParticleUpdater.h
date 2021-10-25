#pragma once
#include "XYZ/Utils/DataStructures/ThreadPass.h"
#include "XYZ/Scene/SceneEntity.h"
#include "ParticleDataBuffer.h"

#include <glm/glm.hpp>

namespace XYZ {


	class TimeUpdater
	{
	public:
		TimeUpdater();
		void UpdateParticles(float timeStep, ParticleDataBuffer* data) const;

		bool m_Enabled;
	};

	class PositionUpdater
	{
	public:
		PositionUpdater();
		void UpdateParticles(float timeStep, ParticleDataBuffer* data) const;

		bool m_Enabled;
	};


	class LightUpdater
	{
	public:
		LightUpdater();

		void UpdateParticles(float timeStep, ParticleDataBuffer* data) const;

		SceneEntity	m_LightEntity;
		SceneEntity	m_TransformEntity;
		uint32_t	m_MaxLights;
		bool		m_Enabled;
	};
}