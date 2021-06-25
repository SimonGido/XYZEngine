#include "stdafx.h"
#include "ParticleSystemCPU.h"

#include "XYZ/Core/Application.h"

namespace XYZ {

	ParticleSystemCPU::ParticleSystemCPU(uint32_t maxParticles)
		:
		m_Renderer(maxParticles)
	{
		m_SingleThreadPass = std::make_shared<SingleThreadPass>(maxParticles);
		m_ThreadPass = std::make_shared<ThreadPass<DoubleThreadPass>>();
		{
			ScopedLockReference<DoubleThreadPass> write = m_ThreadPass->Write();
			write.Get().RenderData.resize(maxParticles);
		}
		{
			ScopedLockReference<DoubleThreadPass> read = m_ThreadPass->Read();
			read.Get().RenderData.resize(maxParticles);
		}
	}

	ParticleSystemCPU::~ParticleSystemCPU()
	{
		m_SingleThreadPass->Play = false;
	}

	void ParticleSystemCPU::Update(Timestep ts, const glm::mat4& transform)
	{	
		{
			ScopedLockReference<DoubleThreadPass> val = m_ThreadPass->Read();
			m_Renderer.InstanceCount = val.Get().InstanceCount;
			m_Renderer.InstanceVBO->Update(val.Get().RenderData.data(), m_Renderer.InstanceCount * sizeof(ParticleRenderData));
		}
		{
			std::scoped_lock lock(m_SingleThreadPass->Mutex);
			for (auto updater : m_SingleThreadPass->Updaters)
			{
				updater->Update();
			}
		}
	}

	struct TimeMeasure
	{
		TimeMeasure()
		{
			m_Start = std::chrono::high_resolution_clock::now();
		}

		float Seconds()
		{
			auto end = std::chrono::high_resolution_clock::now();
			auto temp = std::chrono::duration_cast<std::chrono::microseconds>(end - m_Start).count();
			return (float)temp * 0.000001f;
		}

	private:
		std::chrono::time_point<std::chrono::high_resolution_clock> m_Start;
	};

	void ParticleSystemCPU::Play()
	{
		if (!m_SingleThreadPass->Play)
		{
			m_SingleThreadPass->Play = true;
			auto singleThreadPass = m_SingleThreadPass;
			auto threadPass = m_ThreadPass;
			Application::Get().GetThreadPool().PushJob<void>([singleThreadPass, threadPass]() {

				float timestep = 0.0f;			
				while (singleThreadPass->Play)
				{
					TimeMeasure timer;
					{
						std::scoped_lock lock(singleThreadPass->Mutex);
						ScopedLockReference<DoubleThreadPass> val = threadPass->Write();
						for (auto generator : singleThreadPass->Generators)
						{
							uint32_t startId = singleThreadPass->Particles.GetAliveParticles();
							generator->Generate(&singleThreadPass->Particles, startId, timestep);
						}
						for (auto updater : singleThreadPass->Updaters)
						{
							updater->UpdateParticles(timestep, &singleThreadPass->Particles);
						}
						uint32_t endId = singleThreadPass->Particles.GetAliveParticles();
						for (uint32_t i = 0; i < endId; ++i)
						{
							auto& particle = singleThreadPass->Particles.m_Particle[i];
							val.Get().RenderData[i] = ParticleRenderData{
								particle.Color,
								singleThreadPass->Particles.m_TexCoord[i],
								glm::vec2(particle.Position.x, particle.Position.y),
								singleThreadPass->Particles.m_Size[i],
								singleThreadPass->Particles.m_Rotation[i]
							};
						}
						val.Get().InstanceCount = endId;
					}
					timestep = timer.Seconds();
					threadPass->AttemptSwap();		
				}
			});
		}
	}
	void ParticleSystemCPU::Stop()
	{
		m_SingleThreadPass->Play.store(false);
	}
	void ParticleSystemCPU::AddParticleUpdate(ParticleUpdater* updater)
	{
		std::scoped_lock lock(m_SingleThreadPass->Mutex);
		m_SingleThreadPass->Updaters.push_back(updater);
	}
	void ParticleSystemCPU::AddGenerator(ParticleGenerator* generator)
	{
		std::scoped_lock lock(m_SingleThreadPass->Mutex);
		m_SingleThreadPass->Generators.push_back(generator);
	}
	ParticleSystemCPU::DoubleThreadPass::DoubleThreadPass()
		:
		InstanceCount(0)
	{
	}
	ParticleSystemCPU::DoubleThreadPass::DoubleThreadPass(uint32_t maxParticles)
		:
		InstanceCount(0)
	{
		RenderData.resize(maxParticles);
	}
	ParticleSystemCPU::SingleThreadPass::SingleThreadPass(uint32_t maxParticles)
		:
		Particles(maxParticles),
		Play(false)
	{
	}
	ParticleSystemCPU::SingleThreadPass::~SingleThreadPass()
	{
		for (auto updater : Updaters)
			delete updater;

		for (auto generator : Generators)
			delete generator;
	}
}