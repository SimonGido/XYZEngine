#include "stdafx.h"
#include "ParticleEffect.h"
#include "XYZ/Renderer/Renderer2D.h"
#include "XYZ/Renderer/Renderer.h"


namespace XYZ {


	ParticleEffect::ParticleEffect(const ParticleEffectConfiguration& config, const ParticleLayoutConfiguration& layout)
		: 
		m_VertexArray(VertexArray::Create()),
		m_IndirectBuffer(IndirectBuffer::Create(nullptr, sizeof(DrawElementsIndirectCommand))),
		m_Counter(AtomicCounter::Create(1)),
		m_Config(config),
		m_Layout(layout)
	{

		if (m_Config.MaxParticles > sc_MaxParticlesPerEffect)
		{
			XYZ_LOG_WARN("Max number of particles per effect is ", sc_MaxParticlesPerEffect);
			m_Config.MaxParticles = sc_MaxParticlesPerEffect;
		}
		m_DefaultVertexData.Allocate(m_Config.MaxParticles * m_Layout.ParticleVertexLayout.GetStride());
		m_DefaultInformationData.Allocate(m_Config.MaxParticles * m_Layout.ParticleInformationLayout.GetStride());

		float vert[20] = {
			-0.5f, -0.5f, 0.0f, 0.0f, 0.0f,
			 0.5f, -0.5f, 0.0f, 1.0f, 0.0f,
			 0.5f,  0.5f, 0.0f, 1.0f, 1.0f,
			-0.5f,  0.5f, 0.0f, 0.0f, 1.0f
		};
		Ref<VertexBuffer> squareVBpar;
		squareVBpar = XYZ::VertexBuffer::Create(vert, sizeof(vert));
		squareVBpar->SetLayout(m_Layout.ParticleVertexLayout);
		m_VertexArray->AddVertexBuffer(squareVBpar);

		m_VertexStorage = ShaderStorageBuffer::Create(m_Config.MaxParticles * m_Layout.ParticleVertexLayout.GetStride());
		m_VertexStorage->SetLayout(m_Layout.ParticleInformationLayout);
		m_PropsStorage = ShaderStorageBuffer::Create(m_Config.MaxParticles * m_Layout.ParticleInformationLayout.GetStride());
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

	ParticleEffect::~ParticleEffect()
	{
	}

	void ParticleEffect::Restart()
	{
		m_Counter->Reset();
		m_EmittedParticles = 0.0f;

		m_VertexStorage->Update(m_DefaultVertexData, m_Config.MaxParticles * m_Layout.ParticleVertexLayout.GetStride(), 0);
		m_PropsStorage->Update(m_DefaultInformationData, m_Config.MaxParticles * m_Layout.ParticleInformationLayout.GetStride(), 0);
	}

	void ParticleEffect::Update(Timestep ts)
	{
		float raise = m_Config.Rate * ts;
		if (m_EmittedParticles + raise <= m_Config.MaxParticles)
			m_EmittedParticles += raise;
		
		int emitted = (int)std::ceil(m_EmittedParticles);

		m_PropsStorage->BindRange(0, emitted * sizeof(ParticleInformation), PROPS_BINDING);
		m_VertexStorage->BindRange(0, emitted * sizeof(ParticleVertex), VERTEX_BINDING);

		m_PlayTime += ts;
	}

	void ParticleEffect::SetParticles(void* vertexBuffer, void* particleInfo)
	{
		m_DefaultVertexData.Write(vertexBuffer, m_Config.MaxParticles * m_Layout.ParticleVertexLayout.GetStride());
		m_DefaultInformationData.Write(particleInfo, m_Config.MaxParticles * m_Layout.ParticleInformationLayout.GetStride());

		m_DefaultVertexData = ByteBuffer::Copy(vertexBuffer, m_Config.MaxParticles * m_Layout.ParticleVertexLayout.GetStride());
		m_DefaultInformationData = ByteBuffer::Copy(particleInfo, m_Config.MaxParticles * m_Layout.ParticleInformationLayout.GetStride());

		m_VertexStorage->Update(vertexBuffer, m_Config.MaxParticles * m_Layout.ParticleVertexLayout.GetStride(), 0);
		m_PropsStorage->Update(particleInfo, m_Config.MaxParticles * m_Layout.ParticleInformationLayout.GetStride(), 0);
	}

	void ParticleEffect::SetParticlesRange(void* vertexBuffer, void* particleInfo, uint32_t offset, uint32_t count)
	{
		uint32_t vertexSize = m_Layout.ParticleVertexLayout.GetStride();
		uint32_t infoSize = m_Layout.ParticleInformationLayout.GetStride();

		m_DefaultVertexData.Write(vertexBuffer, count * vertexSize, offset * vertexSize);
		m_DefaultInformationData.Write(particleInfo, count * infoSize, offset * infoSize);

		XYZ_ASSERT(offset + count <= m_Config.MaxParticles, "Attempting to set particles out of range");
		m_VertexStorage->Update(vertexBuffer, count * vertexSize, offset * vertexSize);
		m_PropsStorage->Update(particleInfo, count * infoSize, offset * infoSize);
	}

	void ParticleEffect::GetParticles(void* vertexBuffer, void* particleInfo)
	{
		uint32_t vertexSize = m_Layout.ParticleVertexLayout.GetStride();
		uint32_t infoSize = m_Layout.ParticleInformationLayout.GetStride();
		m_VertexStorage->GetSubData(vertexBuffer, m_Config.MaxParticles * vertexSize, 0);
		m_PropsStorage->GetSubData(particleInfo, m_Config.MaxParticles * infoSize, 0);
	}

	void ParticleEffect::GetParticlesRange(void* vertexBuffer, void* particleInfo, uint32_t offset, uint32_t count)
	{
		XYZ_ASSERT(offset + count <= m_Config.MaxParticles, "Attempting to get particles out of range");
		uint32_t vertexSize = m_Layout.ParticleVertexLayout.GetStride();
		uint32_t infoSize = m_Layout.ParticleInformationLayout.GetStride();
		m_VertexStorage->GetSubData(vertexBuffer, count * vertexSize, offset * vertexSize);
		m_PropsStorage->GetSubData(particleInfo,  count * infoSize, offset * infoSize);
	}

	void ParticleEffect::SetLayout(const ParticleLayoutConfiguration& config)
	{
		XYZ_ASSERT(false, "Rebuild buffers");
		m_Layout = config;
	}

	void ParticleEffect::SetConfiguration(const ParticleEffectConfiguration& config)
	{
		XYZ_ASSERT(m_Config.MaxParticles == config.MaxParticles, "Resize buffers");
		m_Config = config;
	}


	ParticleLayoutConfiguration::ParticleLayoutConfiguration()
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
	}

	ParticleLayoutConfiguration::ParticleLayoutConfiguration(const BufferLayout& particleVertexLayout, const BufferLayout& particleInformationlayout)
		:
		ParticleVertexLayout(particleVertexLayout),
		ParticleInformationLayout(particleInformationlayout)
	{
	}

	ParticleEffectConfiguration::ParticleEffectConfiguration(uint32_t maxParticles, float rate)
		:
		MaxParticles(maxParticles), Rate(rate)
	{
	}

}