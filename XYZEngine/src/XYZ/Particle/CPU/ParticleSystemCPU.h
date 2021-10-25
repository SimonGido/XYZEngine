#pragma once
#include "XYZ/Utils/DataStructures/ThreadPass.h"

#include "XYZ/Core/Timestep.h"
#include "XYZ/Core/Ref.h"
#include "ParticleDataBuffer.h"
#include "ParticleUpdater.h"
#include "ParticleGenerator.h"
#include "ParticleRenderer.h"
#include "ParticleEmitter.h"

#include <glm/glm.hpp>

#include <mutex>

namespace XYZ {

	class SceneRenderer;
	class ParticleSystemCPU : public RefCount
	{
	public:
		struct UpdateData
		{
			TimeUpdater		   m_TimeUpdater;
			PositionUpdater	   m_PositionUpdater;
			LightUpdater	   m_LightUpdater;

			// TODO: might be better to have std::vector<Updater*> and push/erase enabled/disabled updaters
		};
		
	public:
		ParticleSystemCPU(uint32_t maxParticles);
		~ParticleSystemCPU();

		void Update(Timestep ts);
		void SubmitLights(Ref<SceneRenderer> renderer);
		void Play();
		void Stop();

		
		ScopedLock<ParticleDataBuffer>	   GetParticleData();
		ScopedLockRead<ParticleDataBuffer> GetParticleDataRead() const;

		ScopedLock<UpdateData>	   GetUpdateData();
		ScopedLockRead<UpdateData> GetUpdateDataRead() const;

		ScopedLock<ParticleEmitterCPU>	   GetEmitData();
		ScopedLockRead<ParticleEmitterCPU>   GetEmitDataRead() const;

		Ref<ParticleRendererCPU> m_Renderer;
	private:
		void particleThreadUpdate(float timestep);
		void update(Timestep timestep, ParticleDataBuffer& particles);
		void emit(Timestep timestep, ParticleDataBuffer& particles);
		void buildRenderData(ParticleDataBuffer& particles);
		
	private:
		struct RenderData
		{
			RenderData();				
			RenderData(uint32_t maxParticles);
						
			std::vector<ParticleRenderData> m_RenderParticleData;
			uint32_t						m_InstanceCount;
		};
	
		SingleThreadPass<ParticleDataBuffer> m_ParticleData;
		SingleThreadPass<UpdateData>		 m_UpdateThreadPass;	
		SingleThreadPass<ParticleEmitterCPU> m_EmitThreadPass;
		ThreadPass<RenderData>				 m_RenderThreadPass;
		ThreadPass<std::vector<glm::vec3>>   m_LightPass;
		bool								 m_Play;
	};

}