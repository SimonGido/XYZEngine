#pragma once
#include "XYZ/Utils/DataStructures/ThreadPass.h"
#include "XYZ/Scene/SceneEntity.h"
#include "ParticleDataBuffer.h"

#include <glm/glm.hpp>

namespace XYZ {

	class MainModule
	{
	public:
		MainModule();
		void UpdateParticles(float ts, ParticleDataBuffer& data) const;

		void SetEnabled(bool enabled);
		bool IsEnabled() const { return m_Enabled; }

	private:
		bool m_Enabled;
	};

	class LightModule
	{
	public:
		LightModule();

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

	class TextureAnimationModule
	{
	public:
		TextureAnimationModule();

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

	class CollisionModule
	{
	public:
		CollisionModule();

		void Generate(ParticleDataBuffer& data, uint32_t startId, uint32_t endId);
		void UpdateParticles(ParticleDataBuffer& data) const;
		void SetPhysicsWorld(b2World* world);
		void SetMaxParticles(uint32_t count);

		void SetEnabled(bool enabled);
		bool IsEnabled() const { return m_Enabled; }
	private:
		void generateBodies();
		void destroyBodies();
	private:
		std::vector<b2Body*> m_Bodies;
		b2World*			 m_PhysicsWorld;
		uint32_t			 m_MaxParticles;
		bool				 m_Enabled;
	};
}