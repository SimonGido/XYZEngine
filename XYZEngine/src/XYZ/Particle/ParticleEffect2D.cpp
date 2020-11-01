#include "stdafx.h"
#include "ParticleEffect2D.h"
#include "XYZ/Renderer/Renderer2D.h"
#include "XYZ/Renderer/Renderer.h"


namespace XYZ {


	ParticleEffect2D::ParticleEffect2D(const ParticleEffectConfiguration& config)
		: 
		m_VertexArray(VertexArray::Create()),
		m_IndirectBuffer(IndirectBuffer::Create(nullptr, sizeof(DrawElementsIndirectCommand))),
		m_Counter(AtomicCounter::Create(1)),
		m_Configuration(config)
	{

		if (config.MaxParticles > sc_MaxParticlesPerEffect)
		{
			XYZ_LOG_WARN("Max number of particles per effect is ", sc_MaxParticlesPerEffect);
			m_Configuration.MaxParticles = sc_MaxParticlesPerEffect;
		}

		

		BufferLayout buflayout = {
				{2, ShaderDataComponent::Float4, "a_IColor",		  1},
				{3, ShaderDataComponent::Float4, "a_IPosition",		  1},
				{4, ShaderDataComponent::Float2, "a_ITexCoordOffset", 1},
				{5, ShaderDataComponent::Float2, "a_ISize",			  1},
				{6, ShaderDataComponent::Float,  "a_IAngle",		  1},
				{7, ShaderDataComponent::Float,  "a_IAlignment",	  1},
				{8, ShaderDataComponent::Float,  "a_IAlignment2",	  1},
				{9, ShaderDataComponent::Float,  "a_IAlignment3",	  1}
		};
		m_VertexStorage = ShaderStorageBuffer::Create((uint32_t)m_Configuration.MaxParticles * (uint32_t)sizeof(ParticleVertex));
		m_VertexStorage->SetLayout(buflayout);
		m_VertexArray->AddShaderStorageBuffer(m_VertexStorage);
		
		float vert[20] = {
			-0.5f, -0.5f, 0.0f, 0.0f, 0.0f,
			 0.5f, -0.5f, 0.0f, 1.0f, 0.0f,
			 0.5f,  0.5f, 0.0f, 1.0f, 1.0f,
			-0.5f,  0.5f, 0.0f, 0.0f, 1.0f
		};
		Ref<VertexBuffer> squareVBpar;
		squareVBpar = XYZ::VertexBuffer::Create(vert, sizeof(vert));
		squareVBpar->SetLayout({
			{  0, XYZ::ShaderDataComponent::Float3, "a_Position" },
			{  1, XYZ::ShaderDataComponent::Float2, "a_TexCoord" }
			});

		m_VertexArray->AddVertexBuffer(squareVBpar);
		m_PropsStorage = ShaderStorageBuffer::Create((uint32_t)m_Configuration.MaxParticles * (uint32_t)sizeof(ParticleInformation));

		uint32_t squareIndpar[] = { 0, 1, 2, 2, 3, 0 };
		Ref<XYZ::IndexBuffer> squareIBpar;
		squareIBpar = XYZ::IndexBuffer::Create(squareIndpar, sizeof(squareIndpar) / sizeof(uint32_t));
		m_VertexArray->SetIndexBuffer(squareIBpar);

	
		m_VertexStorage->BindBase(VERTEX_BINDING);
		m_PropsStorage->BindBase(PROPS_BINDING);
		m_IndirectBuffer->BindBase(INDIRECT_BINDING);
		m_Counter->BindBase(COUNTER_BINDING);
	}

	ParticleEffect2D::~ParticleEffect2D()
	{
	}

	void ParticleEffect2D::Update(Timestep ts)
	{
		float raise = m_Configuration.Rate * ts;
		if (m_EmittedParticles + raise <= m_Configuration.MaxParticles)
			m_EmittedParticles += raise;
		
		int emitted = (int)std::floor(m_EmittedParticles);

		m_PropsStorage->BindRange(0, emitted * sizeof(ParticleInformation), PROPS_BINDING);
		m_VertexStorage->BindRange(0, emitted * sizeof(ParticleVertex), VERTEX_BINDING);

		ParticleVertex* data = new ParticleVertex[m_Configuration.MaxParticles];

		m_VertexStorage->GetSubData(data, m_Configuration.MaxParticles * sizeof(ParticleVertex));
		for (int i = 0; i < 10; ++i)
		{
			std::cout << data[i].Color.y << std::endl;
		}

		m_PlayTime += ts;
	}

	void ParticleEffect2D::SetParticles(ParticleVertex* vertexBuffer, ParticleInformation* particleInfo)
	{
		m_VertexStorage->Update(vertexBuffer, m_Configuration.MaxParticles * sizeof(ParticleVertex), 0);
		m_PropsStorage->Update(particleInfo,  m_Configuration.MaxParticles * sizeof(ParticleInformation), 0);
	}

	void ParticleEffect2D::SetParticlesRange(ParticleVertex* vertexBuffer, ParticleInformation* particleInfo, uint32_t offset, uint32_t count)
	{
		XYZ_ASSERT(offset + count <= m_Configuration.MaxParticles, "Attempting to set particles out of range");
		m_VertexStorage->Update(vertexBuffer, count * sizeof(ParticleVertex), offset * sizeof(ParticleVertex));
		m_PropsStorage->Update(particleInfo, count * sizeof(ParticleInformation), offset * sizeof(ParticleInformation));
	}

	void ParticleEffect2D::SetParticlesRangeTest(void* vertexBuffer, void* particleInfo, uint32_t offset, uint32_t count)
	{
		XYZ_ASSERT(offset + count <= m_Configuration.MaxParticles, "Attempting to set particles out of range");
		
		m_PropsStorage->BindRange(0, count * sizeof(ParticleInformation), PROPS_BINDING);
		m_VertexStorage->BindRange(0, count * sizeof(ParticleVertex), VERTEX_BINDING);

		m_PropsStorage->Update(particleInfo, count * sizeof(ParticleInformation), offset * sizeof(ParticleInformation));
		m_VertexStorage->Update(vertexBuffer, count * sizeof(ParticleVertex), offset * sizeof(ParticleVertex));
	}

	void ParticleEffect2D::GetParticles(ParticleVertex* vertexBuffer, ParticleInformation* particleInfo)
	{
		m_VertexStorage->GetSubData(vertexBuffer, m_Configuration.MaxParticles * sizeof(ParticleVertex), 0);
		m_PropsStorage->GetSubData(particleInfo,  m_Configuration.MaxParticles * sizeof(ParticleInformation), 0);
	}

	void ParticleEffect2D::GetParticlesRange(ParticleVertex* vertexBuffer, ParticleInformation* particleInfo, uint32_t offset, uint32_t count)
	{
		XYZ_ASSERT(offset + count <= m_Configuration.MaxParticles, "Attempting to get particles out of range");
		m_VertexStorage->GetSubData(vertexBuffer, count * sizeof(ParticleVertex), offset * sizeof(ParticleVertex));
		m_PropsStorage->GetSubData(particleInfo, count * sizeof(ParticleInformation), offset * sizeof(ParticleInformation));
	}

}