#include "stdafx.h"
#include "ParticleSystem.h"


namespace XYZ {
    ParticleConfiguration::ParticleConfiguration()
        :
        ColorRatio(1.0f),
        SizeRatio(1.0f),
        VelocityRatio(1.0f),
        MaxParticles(50),
        Rate(2.0f),
        Gravity(0.0f),
        Speed(1.0f),
        Time(1.0f),
        Repeat(false)
    {
    }

    ParticleSystem::ParticleSystem()
        :
        m_VertexArray(VertexArray::Create()),
        m_IndirectBuffer(IndirectBuffer::Create(nullptr, sizeof(DrawElementsIndirectCommand))),
        m_DeadCounter(AtomicCounter::Create(1)),
        m_EmittedParticles(0.0f),
        m_PlayTime(0.0f)
    {
    }
    void ParticleSystem::Reset()
    {
        m_DeadCounter->Reset();
        m_EmittedParticles = 0.0f;
    }
    void ParticleSystem::Update(Timestep ts)
    {
        float raise = m_Config.Rate * ts;
        if (m_EmittedParticles + raise <= m_Config.MaxParticles)
            m_EmittedParticles += raise;

        int emitted = (int)std::ceil(m_EmittedParticles);

        m_DataStorage->BindRange(0, emitted * sizeof(ParticleData), 0);
        m_SpecsStorage->BindRange(0, emitted * sizeof(ParticleSpecification), 1);

        m_PlayTime += ts;
    }
    void ParticleSystem::SetParticles(ParticleData* dataBuffer, ParticleSpecification* specsBuffer)
    {
        m_DataStorage->Update(dataBuffer, m_Config.MaxParticles * sizeof(ParticleData), 0);
        m_SpecsStorage->Update(specsBuffer, m_Config.MaxParticles * sizeof(ParticleSpecification), 0);
    }
    void ParticleSystem::SetParticles(ParticleData* dataBuffer, ParticleSpecification* specsBuffer, uint32_t offsetParticles, uint32_t countParticles)
    {
        m_DataStorage->Update(dataBuffer,  countParticles * sizeof(ParticleData), offsetParticles * sizeof(ParticleData));
        m_SpecsStorage->Update(specsBuffer, countParticles * sizeof(ParticleSpecification), offsetParticles * sizeof(ParticleSpecification));
    }
}