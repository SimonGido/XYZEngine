#include "stdafx.h"
#include "ParticleSystemCPU.h"

#include "XYZ/Renderer/Renderer.h"
#include "XYZ/Core/Application.h"

#include <random>

namespace XYZ {

	ParticleSystemCPU::ParticleSystemCPU(uint32_t maxParticles)
		:
		m_MaxParticles(maxParticles),
		m_ParticlesAlive(0),
		m_EmittedParticles(0.0f),
		m_Renderer(maxParticles)
	{
		m_ParticlePool.resize(maxParticles);
	}
	ParticleSystemCPU::~ParticleSystemCPU()
	{
	}
	void ParticleSystemCPU::SetMaxParticles(uint32_t maxParticles)
	{
		m_MaxParticles = maxParticles;
		m_ParticlePool.resize(maxParticles);
		m_Renderer.RenderData.resize(maxParticles);
	}
	void ParticleSystemCPU::Update(Timestep ts)
	{
		Application::GetThreadPool().PushJob<void>([this, ts]() {

			m_EmittedParticles += m_Emission.RateOverTime * ts.GetSeconds();
			if (m_EmittedParticles > 1.0f)
			{
				emitt((uint32_t)m_EmittedParticles);
				m_EmittedParticles = 0.0f;
			}
			uint32_t counter = 0;
			for (size_t i = 0; i < m_ParticlePool.size(); ++i)
			{
				auto& particle = m_ParticlePool[i];
				if (!particle.Alive)
					break;

				if (particle.LifeRemaining <= 0.0f)
				{
					// Copy last alive at the place of dead particle
					particle = m_ParticlePool[m_ParticlesAlive - 1];
					m_ParticlePool[m_ParticlesAlive - 1].Alive = false;
					m_ParticlesAlive--;
					continue;
				}

				particle.LifeRemaining -= ts.GetSeconds();
				particle.Position += particle.Velocity * ts.GetSeconds();
				particle.Rotation += particle.AngularVelocity * ts.GetSeconds();

				m_Renderer.RenderData[counter++] = ParticleRenderData{
					particle.Color,
					particle.TexCoord,
					glm::vec2(particle.Position.x, particle.Position.y),
					particle.Size,
					particle.Rotation
				};
			}

			m_Renderer.InstanceCount = m_ParticlesAlive;
		});
	}
	void ParticleSystemCPU::emitt(uint32_t count)
	{
		for (uint32_t i = 0; i < count && m_ParticlesAlive < m_MaxParticles; ++i)
		{
			std::random_device dev;
			std::mt19937 rng(dev());
			std::uniform_real_distribution<double> dist(-1.0, 1.0); // distribution in range [1, 6]
			ParticleCPU& particle = m_ParticlePool[m_ParticlesAlive];
			particle.Alive = true;
			particle.Size = glm::vec2(0.3f);
			particle.Position = glm::vec3(0.0f);
			particle.Color = glm::vec4(1.0f);
			particle.TexCoord = glm::vec4(0.0f, 0.0f, 1.0f, 1.0f);
			particle.Velocity = glm::vec3(dist(rng), dist(rng), 0.0f);
			particle.AngularVelocity = 25.0f;

			particle.LifeRemaining = 3.0f;
			m_ParticlesAlive++;		
		}
	}

	ParticleRendererCPU::ParticleRendererCPU(uint32_t maxParticles)
		:
		VAO(VertexArray::Create()),
		Mode(RenderMode::Billboard),
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
		
		RenderData.resize(maxParticles);
	}


	void ParticleRendererCPU::Bind()
	{
		if (InstanceCount)
		{			
			InstanceVBO->Update(RenderData.data(), InstanceCount * sizeof(ParticleRenderData));
			VAO->Bind();
			Renderer::DrawInstanced(VAO, InstanceCount);
		}
	}
}