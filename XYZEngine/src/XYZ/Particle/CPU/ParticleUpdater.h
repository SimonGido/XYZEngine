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
		void SetEnable(bool enable);

		bool IsEnabled() const;
	private:
		bool m_Enabled;
	};

	class PositionUpdater
	{
	public:
		PositionUpdater();
		void UpdateParticles(float timeStep, ParticleDataBuffer* data) const;
		void SetEnable(bool enable);
		bool IsEnabled() const;
	private:
		bool m_Enabled;
	};


	class LightUpdater
	{
	public:
		LightUpdater();

		void UpdateParticles(float timeStep, ParticleDataBuffer* data) const;
		void SetEnable(bool enable);

		void SetMaxLights(uint32_t maxLights);	
		void SetLightEntity(const SceneEntity& entity);
		void SetTransformEntity(const SceneEntity& entity);
		
		uint32_t    GetMaxLights() const;
		SceneEntity GetLightEntity() const;
		SceneEntity GetTransformEntity() const;
		bool		IsEnabled() const;

	private:
		SceneEntity					m_LightEntity;
		SceneEntity					m_TransformEntity;
		uint32_t					m_MaxLights;

		bool						m_Enabled;
	};
}