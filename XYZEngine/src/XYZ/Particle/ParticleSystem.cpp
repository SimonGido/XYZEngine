#include "stdafx.h"
#include "ParticleSystem.h"


namespace XYZ {
    ParticleConfiguration::ParticleConfiguration()
        :
        ColorRatio(1.0f),
        SizeRatio(1.0f),
        VelocityRatio(1.0f),
        MaxParticles(5),
        Rate(2.0f),
        Gravity(0.0f),
        Speed(1.0f),
        Time(1.0f),
        Repeat(true)
    {
    }

    ParticleSystem::ParticleSystem(const ParticleConfiguration& config)
        :
        m_Config(config),
        m_VertexArray(VertexArray::Create()),
        m_IndirectBuffer(IndirectBuffer::Create(nullptr, sizeof(DrawElementsIndirectCommand), 3)),
        m_DeadCounter(AtomicCounter::Create(1, 4)),
        m_EmittedParticles(0.0f),
        m_PlayTime(0.0f)
    {
        ParticleVertex quad[4] = {
            ParticleVertex{glm::vec3(-0.5f, -0.5f, 0.0f), glm::vec2(0.0f, 0.0f)},
            ParticleVertex{glm::vec3( 0.5f, -0.5f, 0.0f), glm::vec2(1.0f, 0.0f)},
            ParticleVertex{glm::vec3( 0.5f,  0.5f, 0.0f), glm::vec2(1.0f, 1.0f)},
            ParticleVertex{glm::vec3(-0.5f,  0.5f, 0.0f), glm::vec2(0.0f, 1.0f)}
        };

        Ref<VertexBuffer> squareVBpar;
        squareVBpar = XYZ::VertexBuffer::Create(quad, 4 * sizeof(ParticleVertex));
        squareVBpar->SetLayout({
            { 0, XYZ::ShaderDataComponent::Float3, "a_Position" },
            { 1, XYZ::ShaderDataComponent::Float2, "a_TexCoord" }
        });
        m_VertexArray->AddVertexBuffer(squareVBpar);
        m_DataStorage = ShaderStorageBuffer::Create(m_Config.MaxParticles * sizeof(ParticleData), 1);
        m_DataStorage->SetLayout({
                {2, ShaderDataComponent::Float4, "a_IColor",		  1},
                {3, ShaderDataComponent::Float2, "a_IPosition",		  1},
                {4, ShaderDataComponent::Float2, "a_ITexCoord",       1},
                {5, ShaderDataComponent::Float2, "a_ISize",			  1},
                {6, ShaderDataComponent::Float,  "a_IAngle",		  1},
                {7, ShaderDataComponent::Float,  "a_IAlignment",	  1}
         });
        m_SpecsStorage = ShaderStorageBuffer::Create(m_Config.MaxParticles * sizeof(ParticleSpecification), 2);
        
        m_VertexArray->AddShaderStorageBuffer(m_DataStorage);

        uint32_t squareIndpar[] = { 0, 1, 2, 2, 3, 0 };
        Ref<XYZ::IndexBuffer> squareIBpar;
        squareIBpar = XYZ::IndexBuffer::Create(squareIndpar, sizeof(squareIndpar) / sizeof(uint32_t));
        m_VertexArray->SetIndexBuffer(squareIBpar);
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

        m_DataStorage->BindRange(0, emitted * sizeof(ParticleData), 1);
        m_SpecsStorage->BindRange(0, emitted * sizeof(ParticleSpecification), 2);

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