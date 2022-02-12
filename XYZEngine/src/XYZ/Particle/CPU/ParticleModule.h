#pragma once
#include "XYZ/Utils/DataStructures/ThreadPass.h"
#include "XYZ/Physics/PhysicsWorld2D.h"
#include "ParticleDataBuffer.h"

#include <glm/glm.hpp>

namespace XYZ {


	class Module
	{
	public:
		void SetEnabled(bool enabled) { m_Enabled = enabled; }
		bool IsEnabled() const { return m_Enabled; }

	private:
		bool m_Enabled = true;
	};

	class MainModule : public Module
	{
	public:
		MainModule();
		void UpdateParticles(float ts, ParticleDataBuffer& data);

		const std::vector<uint32_t>& Killed() const { return m_Killed; }
	private:
		std::vector<uint32_t> m_Killed;
	};

	class LightModule : public Module
	{
	public:
		LightModule();

		void UpdateParticles(float ts, ParticleDataBuffer& data);
		void Reset();


		const std::vector<glm::vec3>& GetLights() const { return m_Lights; }

		uint32_t MaxLights;		
	private:
		std::vector<glm::vec3> m_Lights;
	};

	class TextureAnimationModule : public Module
	{
	public:
		TextureAnimationModule();

		void UpdateParticles(float ts, ParticleDataBuffer& data) const;

		glm::ivec2 Tiles;
		uint32_t   StartFrame;
		float      CycleLength;
	
	};

	class RotationOverLife : public Module
	{
	public:
		RotationOverLife();

		void UpdateParticles(float ts, ParticleDataBuffer& data) const;


		glm::vec3 EulerAngles;
		float	  CycleLength;
	};

}