#pragma once
#include "XYZ/Utils/DataStructures/ThreadPass.h"
#include "XYZ/Scene/SceneEntity.h"
#include "ParticleDataBuffer.h"

#include <glm/glm.hpp>

namespace XYZ {

	class MainUpdater
	{
	public:
		MainUpdater();
		void UpdateParticles(float ts, ParticleDataBuffer* data) const;

		bool m_Enabled;
	};

	class LightUpdater
	{
	public:
		LightUpdater();

		void UpdateParticles(float ts, ParticleDataBuffer* data) const;

		SceneEntity	m_LightEntity;
		SceneEntity	m_TransformEntity;
		uint32_t	m_MaxLights;
		bool		m_Enabled;
	};

	class TextureAnimationUpdater
	{
	public:
		TextureAnimationUpdater();

		void UpdateParticles(float ts, ParticleDataBuffer* data) const;

		glm::ivec2 m_Tiles;
		uint32_t   m_StartFrame;
		float      m_CycleLength;
		bool	   m_Enabled;

	};

	class RotationOverLife
	{
	public:
		RotationOverLife();

		void UpdateParticles(float ts, ParticleDataBuffer* data) const;


		glm::vec3 m_EulerAngles;
		float	  m_CycleLength;
		bool	  m_Enabled;
	};
}