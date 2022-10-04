#pragma once
#include "ParticlePool.h"
#include "ParticleSystem.h"

#include "XYZ/Scene/Components.h"
#include "XYZ/Core/ThreadPool.h"

namespace XYZ {

	struct ParticleInstanceData
	{
		glm::vec4 Transform[3];
		glm::vec4 Color;
		glm::vec2 TexOffset;
	};

	struct ParticleLightData
	{
		glm::vec3 Color;
		glm::vec3 Position;
		float	  Radius;
		float     Intensity;
	};

	struct ParticleRenderData
	{
		std::vector<ParticleInstanceData> InstanceData;
		std::vector<ParticleLightData>	  LightData;
		std::vector<uint32_t>			  SystemDataIndices;
	};

	class ParticleScene : public RefCount
	{
	public:
		using ModulesEnabled = std::array<bool, ParticleComponent::NumModules>;

		ParticleScene(uint32_t maxParticles = 1000);

		void Begin(Timestep ts);

		void SubmitSystem(const glm::mat4& transform, Ref<ParticleSystem> system, ModulesEnabled enabledModules);

		void End();
		void EndAsync(ThreadPool& threadPool);
		
		const ParticleRenderData& GetRenderData() const { return m_RenderData;}

		uint32_t GetAliveParticlesCount() const { return m_Pool.GetAliveParticles(); }
	private:
		struct SystemData
		{
			Ref<ParticleSystem> System;
			ModulesEnabled		Enabled;
			glm::mat4			Transform;
		};

		void buildRenderData(ThreadPool& threadPool, SystemData& data, uint32_t& lastEndID);
		void buildRenderData(const glm::mat4& transform, uint32_t startId, uint32_t endId);
	private:
		ParticlePool	   m_Pool;
		ParticleRenderData m_RenderData;
		Timestep		   m_Timestep;


		std::shared_mutex	 m_JobsMutex;
		std::atomic_uint32_t m_JobsCount = 0;
		uint32_t			 m_LastJobsCount = 0;

		std::vector<SystemData>	m_ParticleSystems;
		
		static constexpr uint32_t sc_PerJobCount = 500;
	};
}