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

		float vert[20] = {
			-0.5f, -0.5f, 0.0f, 0.0f, 0.0f,
			 0.5f, -0.5f, 0.0f, 1.0f, 0.0f,
			 0.5f,  0.5f, 0.0f, 1.0f, 1.0f,
			-0.5f,  0.5f, 0.0f, 0.0f, 1.0f
		};
		Ref<VertexBuffer> squareVBpar;
		squareVBpar = XYZ::VertexBuffer::Create(vert, sizeof(vert));
		squareVBpar->SetLayout(m_Configuration.ParticleVertexLayout);
		m_VertexArray->AddVertexBuffer(squareVBpar);

		m_VertexStorage = ShaderStorageBuffer::Create(m_Configuration.MaxParticles * m_Configuration.ParticleVertexLayout.GetStride());
		m_VertexStorage->SetLayout(m_Configuration.ParticleInformationLayout);
		m_PropsStorage = ShaderStorageBuffer::Create(m_Configuration.MaxParticles * m_Configuration.ParticleInformationLayout.GetStride());
		m_VertexArray->AddShaderStorageBuffer(m_VertexStorage);
	
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

		m_PlayTime += ts;
	}

	void ParticleEffect2D::SetParticles(void* vertexBuffer, void* particleInfo)
	{
		m_VertexStorage->Update(vertexBuffer, m_Configuration.MaxParticles * m_Configuration.ParticleVertexLayout.GetStride(), 0);
		m_PropsStorage->Update(particleInfo, m_Configuration.MaxParticles * m_Configuration.ParticleInformationLayout.GetStride(), 0);
	}

	void ParticleEffect2D::SetParticlesRange(void* vertexBuffer, void* particleInfo, uint32_t offset, uint32_t count)
	{
		uint32_t vertexSize = m_Configuration.ParticleVertexLayout.GetStride();
		uint32_t infoSize = m_Configuration.ParticleInformationLayout.GetStride();

		XYZ_ASSERT(offset + count <= m_Configuration.MaxParticles, "Attempting to set particles out of range");
		m_VertexStorage->Update(vertexBuffer, count * vertexSize, offset * vertexSize);
		m_PropsStorage->Update(particleInfo, count * infoSize, offset * infoSize);
	}

	void ParticleEffect2D::GetParticles(void* vertexBuffer, void* particleInfo)
	{
		uint32_t vertexSize = m_Configuration.ParticleVertexLayout.GetStride();
		uint32_t infoSize = m_Configuration.ParticleInformationLayout.GetStride();
		m_VertexStorage->GetSubData(vertexBuffer, m_Configuration.MaxParticles * vertexSize, 0);
		m_PropsStorage->GetSubData(particleInfo, m_Configuration.MaxParticles * infoSize, 0);
	}

	void ParticleEffect2D::GetParticlesRange(void* vertexBuffer, void* particleInfo, uint32_t offset, uint32_t count)
	{
		XYZ_ASSERT(offset + count <= m_Configuration.MaxParticles, "Attempting to get particles out of range");
		uint32_t vertexSize = m_Configuration.ParticleVertexLayout.GetStride();
		uint32_t infoSize = m_Configuration.ParticleInformationLayout.GetStride();
		m_VertexStorage->GetSubData(vertexBuffer, count * vertexSize, offset * vertexSize);
		m_PropsStorage->GetSubData(particleInfo,  count * infoSize, offset * infoSize);
	}

	ParticleEffectConfiguration::ParticleEffectConfiguration(uint32_t maxParticles, uint32_t rate, bool loop)
	{
		ParticleVertexLayout = {
			{  0, XYZ::ShaderDataComponent::Float3, "a_Position" },
			{  1, XYZ::ShaderDataComponent::Float2, "a_TexCoord" }
		};
		ParticleInformationLayout = {
				{2, ShaderDataComponent::Float4, "a_IColor",		  1},
				{3, ShaderDataComponent::Float4, "a_IPosition",		  1},
				{4, ShaderDataComponent::Float2, "a_ITexCoordOffset", 1},
				{5, ShaderDataComponent::Float2, "a_ISize",			  1},
				{6, ShaderDataComponent::Float,  "a_IAngle",		  1},
				{7, ShaderDataComponent::Float,  "a_IAlignment",	  1},
				{8, ShaderDataComponent::Float,  "a_IAlignment2",	  1},
				{9, ShaderDataComponent::Float,  "a_IAlignment3",	  1}
		};

		Rate = rate;
		MaxParticles = maxParticles;
		Loop = loop;
	}

	ParticleEffectConfiguration::ParticleEffectConfiguration(const BufferLayout& particleVertexLayout, const BufferLayout& particleInformationlayout, uint32_t maxParticles, uint32_t rate, bool loop)
		: 
		ParticleVertexLayout(particleVertexLayout),
		ParticleInformationLayout(particleInformationlayout),
		Rate(rate),
		MaxParticles(maxParticles),
		Loop(loop)
	{
	}

}