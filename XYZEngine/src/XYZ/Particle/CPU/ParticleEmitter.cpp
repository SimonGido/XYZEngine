#include "stdafx.h"
#include "ParticleEmitter.h"

namespace XYZ {
	ParticleEmitterCPU::ParticleEmitterCPU()
		:
		m_EmitRate(10.0f),
		m_EmittedParticles(0.0f)
	{
		m_Generators.push_back(&m_ShapeGenerator);
		m_Generators.push_back(&m_LifeGenerator);
		m_Generators.push_back(&m_RandomVelocityGenerator);
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

	void ParticleEmitterCPU::AddCustomGenerator(ParticleGenerator* generator)
	{
		std::scoped_lock lock(m_Mutex);
		if (generator->IsEnabled())
			m_Generators.push_back(generator);
	}

	void ParticleEmitterCPU::RemoveCustomGenerator(const ParticleGenerator* generator)
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
	void ParticleEmitterCPU::SetShapeGenerator(const ParticleShapeGenerator& generator)
	{
		std::scoped_lock lock(m_Mutex);
		m_ShapeGenerator = generator;
		removeGenerator(&m_ShapeGenerator);
		if (m_ShapeGenerator.IsEnabled())
			m_Generators.push_back(&m_ShapeGenerator);
	}
	void ParticleEmitterCPU::SetLifeGenerator(const ParticleLifeGenerator& generator)
	{
		std::scoped_lock lock(m_Mutex);
		m_LifeGenerator = generator;
		if (m_LifeGenerator.IsEnabled())
			m_Generators.push_back(&m_LifeGenerator);
		else
			removeGenerator(&m_LifeGenerator);
	}
	void ParticleEmitterCPU::SetRandomVelocityGenerator(const ParticleRandomVelocityGenerator& generator)
	{
		std::scoped_lock lock(m_Mutex);
		m_RandomVelocityGenerator = generator;
		if (m_RandomVelocityGenerator.IsEnabled())
			m_Generators.push_back(&m_RandomVelocityGenerator);
		else
			removeGenerator(&m_RandomVelocityGenerator);
	}
	ParticleShapeGenerator ParticleEmitterCPU::GetShapeGenerator() const
	{
		std::scoped_lock lock(m_Mutex);
		return m_ShapeGenerator;
	}
	ParticleLifeGenerator ParticleEmitterCPU::GetLifeGenerator() const
	{
		std::scoped_lock lock(m_Mutex);
		return m_LifeGenerator;
	}
	ParticleRandomVelocityGenerator ParticleEmitterCPU::GetRandomVelocityGenerator() const
	{
		std::scoped_lock lock(m_Mutex);
		return m_RandomVelocityGenerator;
	}
	void ParticleEmitterCPU::removeGenerator(const ParticleGenerator* generator)
	{
		size_t counter = 0;
		for (auto gen : m_Generators)
		{
			if (gen == generator)
			{
				m_Generators.erase(m_Generators.begin() + counter);
				return;
			}
			counter++;
		}
	}
}