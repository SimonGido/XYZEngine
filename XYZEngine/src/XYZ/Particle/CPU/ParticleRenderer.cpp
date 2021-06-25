#include "stdafx.h"
#include "ParticleRenderer.h"
#include "XYZ/Renderer/Renderer.h"


namespace XYZ {
	ParticleRendererCPU::ParticleRendererCPU(uint32_t maxParticles)
		:
		VAO(VertexArray::Create()),
		InstanceCount(0)
	{
		glm::vec3 quad[4] = {
			glm::vec3(-0.5f, -0.5f, 0.0f),
			glm::vec3( 0.5f, -0.5f, 0.0f),
			glm::vec3( 0.5f,  0.5f, 0.0f),
			glm::vec3(-0.5f,  0.5f, 0.0f)
		};

		Ref<VertexBuffer> squareVBpar;
		squareVBpar = XYZ::VertexBuffer::Create(quad, 4 * sizeof(glm::vec3));
		squareVBpar->SetLayout({
			{ 0, XYZ::ShaderDataComponent::Float3, "a_Position" }
			});
		VAO->AddVertexBuffer(squareVBpar);

		InstanceVBO = VertexBuffer::Create(maxParticles * sizeof(ParticleRenderData));
		InstanceVBO->SetLayout({
			{ 1, XYZ::ShaderDataComponent::Float4, "a_IColor",    1 },
			{ 2, XYZ::ShaderDataComponent::Float4, "a_ITexCoord", 1 },
			{ 3, XYZ::ShaderDataComponent::Float2, "a_IPosition", 1 },
			{ 4, XYZ::ShaderDataComponent::Float2, "a_ISize",     1 },
			{ 5, XYZ::ShaderDataComponent::Float,  "a_IAngle",    1 }
			});
		VAO->AddVertexBuffer(InstanceVBO);

		uint32_t squareIndpar[] = { 0, 1, 2, 2, 3, 0 };
		Ref<XYZ::IndexBuffer> squareIBpar;
		squareIBpar = XYZ::IndexBuffer::Create(squareIndpar, sizeof(squareIndpar) / sizeof(uint32_t));
		VAO->SetIndexBuffer(squareIBpar);
	}

	void ParticleRendererCPU::Bind() const
	{
		VAO->Bind();
		Renderer::DrawInstanced(VAO, InstanceCount);
	}
}