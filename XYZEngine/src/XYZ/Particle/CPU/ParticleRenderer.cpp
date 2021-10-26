#include "stdafx.h"
#include "ParticleRenderer.h"
#include "XYZ/Renderer/Renderer.h"


namespace XYZ {
	ParticleRendererCPU::ParticleRendererCPU(uint32_t maxParticles)
		:
		m_InstanceCount(0),
		m_MaxParticles(maxParticles)
	{
		createBuffers();
	}

	void ParticleRendererCPU::Bind() const
	{
		m_VAO->Bind();
		Renderer::DrawInstanced(m_VAO, m_InstanceCount);
	}
	void ParticleRendererCPU::SetCustomVBOLayout(const BufferLayout& layout)
	{
		m_CustomVBO = VertexBuffer::Create(m_MaxParticles * layout.GetStride());
		m_CustomVBO->SetLayout(layout);
		createBuffers();
		m_VAO->AddVertexBuffer(m_CustomVBO);
	}
	void ParticleRendererCPU::createBuffers()
	{
		m_VAO = VertexArray::Create();
		glm::vec3 quad[4] = {
			glm::vec3(-0.5f, -0.5f, 0.0f),
			glm::vec3(0.5f, -0.5f, 0.0f),
			glm::vec3(0.5f,  0.5f, 0.0f),
			glm::vec3(-0.5f,  0.5f, 0.0f)
		};

		Ref<VertexBuffer> squareVBpar;
		squareVBpar = XYZ::VertexBuffer::Create(quad, 4 * sizeof(glm::vec3));
		squareVBpar->SetLayout({
			{ 0, XYZ::ShaderDataComponent::Float3, "a_Position" }
			});
		m_VAO->AddVertexBuffer(squareVBpar);

		m_InstanceVBO = VertexBuffer::Create(m_MaxParticles * sizeof(ParticleRenderData));
		m_InstanceVBO->SetLayout({
			{ 1, XYZ::ShaderDataComponent::Float4, "a_IColor",    1 },
			{ 2, XYZ::ShaderDataComponent::Float4, "a_ITexCoord", 1 },
			{ 3, XYZ::ShaderDataComponent::Float2, "a_IPosition", 1 },
			{ 4, XYZ::ShaderDataComponent::Float2, "a_ISize",     1 },
			{ 5, XYZ::ShaderDataComponent::Float,  "a_IAngle",    1 }
			});
		m_VAO->AddVertexBuffer(m_InstanceVBO);

		uint32_t squareIndpar[] = { 0, 1, 2, 2, 3, 0 };
		Ref<XYZ::IndexBuffer> squareIBpar;
		squareIBpar = XYZ::IndexBuffer::Create(squareIndpar, sizeof(squareIndpar) / sizeof(uint32_t));
		m_VAO->SetIndexBuffer(squareIBpar);
	}
	void ParticleRendererCPU::setMaxParticles(uint32_t maxParticles)
	{
		m_MaxParticles = maxParticles;
		createBuffers();
		if (m_CustomVBO.Raw())
		{
			BufferLayout layout = m_CustomVBO->GetLayout();
			SetCustomVBOLayout(layout);
		}
	}
}