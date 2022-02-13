#pragma once

#include "XYZ/Core/Timestep.h"
#include "ParticleDataBuffer.h"
#include "ParticleGenerator.h"

namespace XYZ {

	
	struct EmitterBurst
	{
		EmitterBurst() = default;
		EmitterBurst(uint32_t count, float time, float probability);

		uint32_t Count = 0;
		float	 Time = 0.0f;
		float    Probability = 1.0f;
	private:
		bool	 m_Called = false;

		friend class ParticleEmitter;
	};

	class ParticleEmitter
	{
	public:
		ParticleEmitter();

		std::pair<uint32_t, uint32_t> Emit(Timestep ts, ParticleDataBuffer& particles);
		float GetEmittedParticles() const { return m_EmittedParticles; }

	
		float							EmitRate;
		float							BurstInterval;
		std::vector<EmitterBurst>		Bursts;

	private:
		uint32_t burstEmit();

	private:
		float m_EmittedParticles;
		float m_PassedTime;
	};
}