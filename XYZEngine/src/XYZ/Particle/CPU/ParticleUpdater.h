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
		void UpdateParticles(float ts, ParticleDataBuffer& data) const;

		void SetEnabled(bool enabled);
		bool IsEnabled() const { return m_Enabled; }

	private:
		bool m_Enabled;
	};

	class LightUpdater
	{
	public:
		LightUpdater();

		void UpdateParticles(float ts, ParticleDataBuffer& data);

		void SetEnabled(bool enabled);
		bool IsEnabled() const { return m_Enabled; }
		const std::vector<glm::vec3>& GetLights() const { return m_Lights; }

		SceneEntity	m_LightEntity;
		SceneEntity	m_TransformEntity;
		
		uint32_t	m_MaxLights;
		
	private:
		std::vector<glm::vec3> m_Lights;
		bool				   m_Enabled;
	};

	class TextureAnimationUpdater
	{
	public:
		TextureAnimationUpdater();

		void UpdateParticles(float ts, ParticleDataBuffer& data) const;

		void SetEnabled(bool enabled);
		bool IsEnabled() const { return m_Enabled; }

		glm::ivec2 m_Tiles;
		uint32_t   m_StartFrame;
		float      m_CycleLength;
		
	private:
		bool m_Enabled;
	};

	class RotationOverLife
	{
	public:
		RotationOverLife();

		void UpdateParticles(float ts, ParticleDataBuffer& data) const;

		void SetEnabled(bool enabled);
		bool IsEnabled() const { return m_Enabled; }

		glm::vec3 m_EulerAngles;
		float	  m_CycleLength;
		
	private:
		bool m_Enabled;
	};
}