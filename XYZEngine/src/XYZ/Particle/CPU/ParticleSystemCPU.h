#pragma once
#include "XYZ/Utils/DataStructures/ThreadPass.h"

#include "XYZ/Core/Timestep.h"
#include "XYZ/Core/Ref.h"
#include "ParticleDataBuffer.h"
#include "ParticleUpdater.h"
#include "ParticleGenerator.h"
#include "ParticleEmitter.h"

#include <glm/glm.hpp>

#include <mutex>

namespace XYZ {

	//TODO: handle 3d particles
	struct ParticleRenderData
	{
		glm::vec4 Color;
		glm::vec4 TexCoord;
		glm::vec2 Position;
		glm::vec2 Size;
		float	  Rotation;
	};

	class SceneRenderer;
	class ParticleSystemCPU
	{
	public:
		struct UpdateData
		{
			TimeUpdater		   m_TimeUpdater;
			PositionUpdater	   m_PositionUpdater;
			LightUpdater	   m_LightUpdater;
		};
		struct RenderData
		{
			RenderData();
			RenderData(uint32_t maxParticles);

			CustomBuffer m_RenderParticleData;
			uint32_t	 m_InstanceCount;
		};

	public:
		ParticleSystemCPU();
		ParticleSystemCPU(uint32_t maxParticles);	
		ParticleSystemCPU(const ParticleSystemCPU& other);
		ParticleSystemCPU(ParticleSystemCPU&& other) noexcept;
		~ParticleSystemCPU();

		ParticleSystemCPU& operator=(const ParticleSystemCPU& other);
		ParticleSystemCPU& operator=(ParticleSystemCPU&& other) noexcept;

		void Update(Timestep ts);
		void SubmitLights(Ref<SceneRenderer> renderer);
		void Play();
		void Stop();
		void SetMaxParticles(uint32_t maxParticles);
		uint32_t GetMaxParticles() const;

		ScopedLock<ParticleDataBuffer>	   GetParticleData();
		ScopedLockRead<ParticleDataBuffer> GetParticleDataRead() const;

		ScopedLock<UpdateData>			   GetUpdateData();
		ScopedLockRead<UpdateData>		   GetUpdateDataRead() const;

		ScopedLock<ParticleEmitterCPU>	   GetEmitter();
		ScopedLockRead<ParticleEmitterCPU> GetEmitterRead() const;

		ScopedLock<RenderData>			   GetRenderData();
		ScopedLockRead<RenderData>		   GetRenderDataRead() const;

	private:
		void particleThreadUpdate(float timestep);
		void update(Timestep timestep, ParticleDataBuffer& particles);
		void emit(Timestep timestep, ParticleDataBuffer& particles);
		void buildRenderData(ParticleDataBuffer& particles);
		
	private:
		
		SingleThreadPass<ParticleDataBuffer> m_ParticleData;
		SingleThreadPass<UpdateData>		 m_UpdateThreadPass;	
		SingleThreadPass<ParticleEmitterCPU> m_EmitThreadPass;
		ThreadPass<RenderData>				 m_RenderThreadPass;
		ThreadPass<std::vector<glm::vec3>>   m_LightPass;
		uint32_t							 m_MaxParticles;
		bool								 m_Play;
	};

}