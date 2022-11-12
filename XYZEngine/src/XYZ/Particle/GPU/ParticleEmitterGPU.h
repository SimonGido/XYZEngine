#pragma once
#include "XYZ/Core/Timestep.h"
#include "ParticleEmitterModuleGPU.h"

#include <glm/glm.hpp>

namespace XYZ {


	class XYZ_API ParticleEmitterGPU : public RefCount
	{
	public:
		ParticleEmitterGPU(uint32_t stride);

		uint32_t Emit(Timestep ts, std::byte* buffer, uint32_t bufferSize);

		float    GetEmitted() const { return m_Emitted; }

		std::vector<Ref<ParticleEmitterModuleGPU>> EmitterModules;
		float EmissionRate;
	
	private:
		uint32_t m_Stride;
		float    m_Emitted;
	};

}