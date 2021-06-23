#pragma once
#include "XYZ/Renderer/VertexArray.h"
#include "XYZ/Renderer/Material.h"
#include "XYZ/Core/Ref.h"
#include "XYZ/Core/Timestep.h"
#include "ParticleCPU.h"

#include <glm/glm.hpp>

namespace XYZ {
	
	struct Emission
	{
		float RateOverTime = 10.0f;
	};

	class ParticleSystemCPU : public RefCount
	{
	public:
		ParticleSystemCPU(uint32_t maxParticles);
		~ParticleSystemCPU();

		void SetMaxParticles(uint32_t maxParticles);

		void Update(Timestep ts);
		
		Emission m_Emission;
	private:
		void emitt(uint32_t count);

	private:
		std::vector<ParticleCPU> m_ParticlePool;
		uint32_t				 m_MaxParticles;
		uint32_t				 m_ParticlesAlive;
		float				     m_EmittedParticles;
	};
}