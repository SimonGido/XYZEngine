#pragma once
#include "XYZ/Utils/DataStructures/ThreadPass.h"
#include "XYZ/Core/Timestep.h"
#include "XYZ/Core/Ref.h"
#include "ParticleData.h"
#include "ParticleUpdater.h"
#include "ParticleGenerator.h"
#include "ParticleRenderer.h"
#include "ParticleEmitter.h"

#include <glm/glm.hpp>

#include <mutex>

namespace XYZ {

	class ParticleSystemCPU : public RefCount
	{
	public:
		ParticleSystemCPU(uint32_t maxParticles);
		~ParticleSystemCPU();

		void Update(Timestep ts);
		void Play();
		void Stop();


		void AddEmitter(const Ref<ParticleEmitterCPU>& emitter);
		void AddUpdater(const Ref<ParticleUpdater>& updater);

		void RemoveEmitter(const Ref<ParticleEmitterCPU>& emitter);
		void RemoveUpdater(const Ref<ParticleUpdater>& updater);

		std::vector<Ref<ParticleUpdater>> GetUpdaters() const;
		std::vector<Ref<ParticleEmitterCPU>> GetEmitters() const;

		Ref<ParticleRendererCPU> m_Renderer;
	private:
		void particleThreadUpdate(float timestep);

	private:
		struct DoubleThreadPass
		{
			DoubleThreadPass();				
			DoubleThreadPass(uint32_t maxParticles);
				
			
			std::vector<ParticleRenderData> RenderData;
			uint32_t						InstanceCount;
		};

		struct SingleThreadPass
		{
			SingleThreadPass(uint32_t maxParticles);

			ParticleDataBuffer					 Particles;
			std::vector<Ref<ParticleUpdater>>	 Updaters;	
			std::vector<Ref<ParticleEmitterCPU>> Emitters;
			mutable std::mutex					 Mutex;
		};

			
		std::shared_ptr<SingleThreadPass>			  m_SingleThreadPass;
		std::shared_ptr<ThreadPass<DoubleThreadPass>> m_ThreadPass;
		
		bool										  m_Play;
	};

}