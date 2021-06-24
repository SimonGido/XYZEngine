#include "stdafx.h"
#include "ParticleSystemCPU.h"

#include "XYZ/Renderer/Renderer.h"
#include "XYZ/Core/Application.h"

#include <random>

namespace XYZ {

	ParticleSystemCPU::ParticleSystemCPU(uint32_t maxParticles)
		:
		m_MaxParticles(maxParticles),
		m_Renderer(maxParticles),
		m_ThreadPass(maxParticles),
		m_Timestep(0.0f),
		m_Playing(false),
		m_ProcessByThread(false)
	{
	}
	ParticleSystemCPU::~ParticleSystemCPU()
	{
	}
	void ParticleSystemCPU::SetMaxParticles(uint32_t maxParticles)
	{
		m_MaxParticles = maxParticles;
		{
			std::scoped_lock<std::mutex> lock(m_ThreadPass.Mutex);
			m_ThreadPass.ParticlePool.resize(maxParticles);
			m_ThreadPass.RenderData.resize(maxParticles);
		}
	}
	void ParticleSystemCPU::Play()
	{
		m_Playing = true;
	}
	void ParticleSystemCPU::Update(Timestep ts, const glm::mat4& transform)
	{
		if (m_Playing)
		{
			m_Timestep += ts;
			if (m_ProcessByThread)
			{
				m_ProcessByThread = false;
				float timeStep = m_Timestep;
				ParticleThreadPass* pass = &m_ThreadPass;
				Application::GetThreadPool().PushJob<void>([this, pass, transform, timeStep]() {

					std::scoped_lock<std::mutex> lock(pass->Mutex);
					m_EmissionModule.Process(*pass, timeStep);
					

					if (pass->EmittedParticles > 1.0f)
					{
						emitt(*pass);
						pass->EmittedParticles = 0.0f;
					}
					uint32_t counter = 0;
					for (size_t i = 0; i < pass->ParticlePool.size(); ++i)
					{
						auto& particle = pass->ParticlePool[i];
						if (!particle.Alive)
							break;

						if (particle.LifeRemaining <= 0.0f)
						{
							// Copy last alive at the place of dead particle
							particle = pass->ParticlePool[pass->ParticlesAlive - 1];
							pass->ParticlePool[pass->ParticlesAlive - 1].Alive = false;
							pass->ParticlesAlive--;
							continue;
						}

						m_VelocityModule.Process(particle, timeStep);
						particle.LifeRemaining -= timeStep;
						particle.Position += particle.Velocity * timeStep;
						particle.Rotation += particle.AngularVelocity * timeStep;

						pass->RenderData[counter++] = ParticleRenderData{
							particle.Color,
							particle.TexCoord,
							glm::vec2(particle.Position.x, particle.Position.y),
							particle.Size,
							particle.Rotation
						};
					}
					pass->InstanceCount = pass->ParticlesAlive;
				});
				m_Timestep = 0.0f;
			}
			attemptSync();
		}
	}
	void ParticleSystemCPU::SetEmissionModule(const EmissionModule& module)
	{
		std::scoped_lock<std::mutex> lock(m_ThreadPass.Mutex);
		m_EmissionModule = module;
	}
	void ParticleSystemCPU::attemptSync()
	{
		if (!m_ProcessByThread && m_ThreadPass.Mutex.try_lock())
		{
			m_Renderer.InstanceCount = m_ThreadPass.InstanceCount;
			m_Renderer.InstanceVBO->Update(m_ThreadPass.RenderData.data(), m_Renderer.InstanceCount * sizeof(ParticleRenderData));
			m_ProcessByThread = true;
			m_ThreadPass.Mutex.unlock();
		}
	}

	void ParticleSystemCPU::emitt(ParticleThreadPass& pass)
	{
		uint32_t count = (uint32_t)pass.EmittedParticles;
		for (uint32_t i = 0; i < count && pass.ParticlesAlive < pass.ParticlePool.size(); ++i)
		{
			std::random_device dev;
			std::mt19937 rng(dev());
			std::uniform_real_distribution<double> dist(-1.0, 1.0); // distribution in range [1, 6]
			ParticleCPU& particle = pass.ParticlePool[pass.ParticlesAlive];
			particle.Alive = true;
			particle.Size = glm::vec2(0.3f);
			particle.Position = glm::vec3(0.0f);
			particle.Color = glm::vec4(1.0f);
			particle.TexCoord = glm::vec4(0.0f, 0.0f, 1.0f, 1.0f);
			particle.Velocity = glm::vec3(dist(rng), dist(rng), 0.0f);
			particle.AngularVelocity = 25.0f;

			particle.LifeRemaining = 3.0f;
			pass.ParticlesAlive++;
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
	}

	void ParticleRendererCPU::Bind() const
	{
		VAO->Bind();
		Renderer::DrawInstanced(VAO, InstanceCount);
	}

}