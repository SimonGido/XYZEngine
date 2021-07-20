#include "stdafx.h"
#include "ParticleEmitter.h"

namespace XYZ {
	ParticleEmitterCPU::ParticleEmitterCPU()
		:
		m_EmitRate(100000.0f),
		m_EmittedParticles(0.0f)
	{
	}
	void ParticleEmitterCPU::Emit(float timeStep, ParticleDataBuffer * data)
	{
		std::scoped_lock lock(m_Mutex);
		m_EmittedParticles += timeStep * m_EmitRate;

		const uint32_t newParticles = (uint32_t)m_EmittedParticles;
		const uint32_t startId = data->GetAliveParticles();
		const uint32_t endId = std::min(startId + newParticles, data->GetMaxParticles() - 1);
		if (newParticles)
			m_EmittedParticles = 0.0f;

		for (auto& gen : m_Generators)
			gen->Generate(data, startId, endId);

		for (uint32_t i = startId; i < endId; ++i)
			data->Wake(i);
	}

	void ParticleEmitterCPU::AddGenerator(const Ref<ParticleGenerator>& generator)
	{
		std::scoped_lock lock(m_Mutex);	
		m_Generators.push_back(generator);
	}

	void ParticleEmitterCPU::RemoveGenerator(const Ref<ParticleGenerator>& generator)
	{
		std::scoped_lock lock(m_Mutex);
		removeGenerator(generator);
	}

	void ParticleEmitterCPU::SetEmitRate(float emitRate)
	{
		std::scoped_lock lock(m_Mutex);
		m_EmitRate = emitRate;
	}
	float ParticleEmitterCPU::GetEmitRate() const
	{
		std::scoped_lock lock(m_Mutex);
		return m_EmitRate;
	}
	void ParticleEmitterCPU::removeGenerator(const Ref<ParticleGenerator>& generator)
	{
		size_t counter = 0;
		for (auto gen : m_Generators)
		{
			if (gen.Raw() == generator.Raw())
			{
				m_Generators.erase(m_Generators.begin() + counter);
				return;
			}
			counter++;
		}
	}
}