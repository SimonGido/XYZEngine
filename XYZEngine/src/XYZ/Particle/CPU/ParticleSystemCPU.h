#pragma once
#include "XYZ/Utils/DataStructures/ThreadPass.h"

#include "XYZ/Core/Timestep.h"
#include "XYZ/Core/Ref.h"
#include "ParticleDataBuffer.h"
#include "ParticleUpdater.h"
#include "ParticleGenerator.h"
#include "ParticleRenderer.h"

#include <glm/glm.hpp>

#include <mutex>

namespace XYZ {

	class SceneRenderer;
	class ParticleSystemCPU : public RefCount
	{
	public:
		struct UpdateData
		{
			TimeUpdater		   TimeUpdater;
			PositionUpdater	   PositionUpdater;
			LightUpdater	   LightUpdater;
		};
		struct EmitData
		{
			ParticleShapeGenerator		    ShapeGenerator;
			ParticleLifeGenerator		    LifeGenerator;
			ParticleRandomVelocityGenerator RandomVelGenerator;
			float							EmittedParticles;
			float							EmitRate = 10.0f;
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

		ScopedLock<EmitData>	   GetEmitData();
		ScopedLockRead<EmitData>   GetEmitDataRead() const;

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
						
			std::vector<ParticleRenderData> RenderParticleData;
			uint32_t						InstanceCount;
		};
	
		SingleThreadPass<ParticleDataBuffer> m_ParticleData;
		SingleThreadPass<UpdateData>		 m_UpdateThreadPass;	
		SingleThreadPass<EmitData>			 m_EmitThreadPass;
		ThreadPass<RenderData>				 m_RenderThreadPass;
		bool								 m_Play;
	};

}