#pragma once
#include "XYZ/Core/Timestep.h"
#include "ParticleData.h"
#include "ParticleUpdater.h"
#include "ParticleGenerator.h"

#include "XYZ/Renderer/RendererCommand.h"
#include "XYZ/Core/Ref.h"
#include "ParticleSystemCPU.h"
#include <glm/glm.hpp>

namespace XYZ {


	class ParticleSystemCPUTest : public RefCount
	{
	public:
		ParticleSystemCPUTest(uint32_t maxParticles);

		void Update(Timestep ts);

		void AddParticleUpdate(ParticleUpdater* updater) { m_Updaters.push_back(updater); }
		void AddGenerator(ParticleGenerator* generator) { m_Generators.push_back(generator); }
		
		ParticleRendererCPU& GetRenderer() { return m_Renderer; }
	private:
		ParticleDataBuffer	m_Particles;
		ParticleRendererCPU	m_Renderer;
		std::vector<ParticleRenderData> m_RenderData;
		std::vector<ParticleUpdater*>   m_Updaters;
		std::vector<ParticleGenerator*> m_Generators;
	};

}