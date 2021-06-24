#pragma once

#include "XYZ/Core/Timestep.h"
#include "ParticleCPU.h"

#include <glm/glm.hpp>

#include <mutex>


namespace XYZ {

	struct ParticleRenderData
	{
		glm::vec4 Color;
		glm::vec4 TexCoord;
		glm::vec2 Position;
		glm::vec2 Size;
		float     Angle;
	};

	struct ParticleThreadPass
	{
		ParticleThreadPass(uint32_t maxParticles);

		std::mutex						Mutex;
		std::vector<ParticleRenderData> RenderData;
		std::vector<ParticleCPU>		ParticlePool;		
		uint32_t					    InstanceCount;
		uint32_t						ParticlesAlive;
		float							EmittedParticles;
	};
	struct EmissionModule
	{
		EmissionModule();

		void Process(ParticleThreadPass& pass, float timeStep);

		float RateOverTime;
		bool  Enabled;
	};

	struct VelocityOverLifeModule
	{
		VelocityOverLifeModule();

		void Process(ParticleCPU& particle, float timeStep);

		glm::vec3 Velocity;
		bool      Enabled;
	};

	//struct LightModule
	//{
	//	LightModule();
	//
	//	void Process(const ParticleCPU& particle, const glm::mat4& transform);
	//
	//	SceneEntity	  LightEntity;
	//	bool		  Enabled;
	//};
}