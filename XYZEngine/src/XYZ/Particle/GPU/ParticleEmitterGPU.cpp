#include "stdafx.h"
#include "ParticleEmitterGPU.h"

#include "ParticleSystemGPU.h"

#include <glm/common.hpp>
#include <glm/gtc/random.hpp>
#include <glm/gtc/constants.hpp>

namespace XYZ {
	DefaultParticleEmitterGPU::DefaultParticleEmitterGPU()
		:
		m_EmissionRate(1.0f),
		m_Emitted(0.0f)
	{
	}
	void DefaultParticleEmitterGPU::Emit(uint32_t count, std::byte* buffer)
	{
		uint32_t offset = 0;
		if (BoxEmitterModule.Enabled)
		{
			for (uint32_t i = 0; i < count; ++i)
			{
				ParticlePropertyGPU* particle = reinterpret_cast<ParticlePropertyGPU*>(&buffer[offset]);
				particle->StartPosition = glm::vec4(BoxEmitterModule.Generate(), 0.0f);
			}
		}
	}
	uint32_t DefaultParticleEmitterGPU::EmitCount(Timestep ts)
	{
		m_Emitted += m_EmissionRate * ts;
		uint32_t count = static_cast<uint32_t>(m_Emitted);
		if (count > 0)
			m_Emitted = 0.0f;

		return count;
	}
	void DefaultParticleEmitterGPU::SetEmissionRate(float rate)
	{
	}
	uint32_t DefaultParticleEmitterGPU::GetStride() const
	{
		return sizeof(ParticlePropertyGPU);
	}
	float DefaultParticleEmitterGPU::GetEmissionRate() const
	{
		return m_EmissionRate;
	}
}