#pragma once
#include "XYZ/Core/Timestep.h"
#include "XYZ/Core/Ref.h"
#include "ParticleData.h"
#include "ParticleUpdater.h"
#include "ParticleGenerator.h"
#include "ParticleRenderer.h"
#include "XYZ/Utils/DataStructures/ThreadPass.h"

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

		void AddParticleUpdate(ParticleUpdater* updater);
		void AddGenerator(ParticleGenerator* generator);

		ParticleRendererCPU& GetRenderer() { return m_Renderer; }
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
			~SingleThreadPass();

			ParticleDataBuffer			    Particles;
			std::vector<ParticleUpdater*>	Updaters;
			std::vector<ParticleGenerator*>	Generators;

			std::atomic<bool>			    Play;
			std::mutex						Mutex;
		};

		ParticleRendererCPU							  m_Renderer;	
		std::shared_ptr<SingleThreadPass>			  m_SingleThreadPass;
		std::shared_ptr<ThreadPass<DoubleThreadPass>> m_ThreadPass;
	};

}