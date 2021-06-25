#include "stdafx.h"
#include "ParticleSystemCPU.h"

#include "XYZ/Core/Application.h"

namespace XYZ {

	struct FrameCap
	{
		FrameCap()
			:
			m_Start(std::chrono::system_clock::now()),
			m_End(std::chrono::system_clock::now())
		{
		}

		float Begin(float ms)
		{
			m_Start = std::chrono::system_clock::now();
			std::chrono::duration<float, std::milli> workTime = m_Start - m_End;
			if (workTime.count() < ms)
			{
				std::chrono::duration<float, std::milli> deltaMs(ms - workTime.count());
				auto delta_ms_duration = std::chrono::duration_cast<std::chrono::milliseconds>(deltaMs);
				std::this_thread::sleep_for(std::chrono::milliseconds(delta_ms_duration.count()));
				return ms;
			}
			return workTime.count() * 0.001f;
		}
		void End()
		{
			m_End = std::chrono::system_clock::now();;
		}

	private:
		std::chrono::system_clock::time_point m_Start;
		std::chrono::system_clock::time_point m_End;
	};
	

	struct TimeMeasure
	{
		TimeMeasure()
			:
			m_Start(std::chrono::system_clock::now()),
			m_End(std::chrono::system_clock::now())
		{
		}

		double Begin()
		{
			m_Start = std::chrono::system_clock::now();
			std::chrono::duration<double, std::milli> workTime = m_Start - m_End;
			return workTime.count();
		}
		void End()
		{
			m_End = std::chrono::system_clock::now();;
		}

	private:
		std::chrono::system_clock::time_point m_Start;
		std::chrono::system_clock::time_point m_End;
	};

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

	void ParticleSystemCPU::Update(Timestep ts)
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

	void ParticleSystemCPU::Play()
	{
		if (!m_SingleThreadPass->Play)
		{
			m_SingleThreadPass->Play = true;
			auto singleThreadPass = m_SingleThreadPass;
			auto threadPass = m_ThreadPass;
			Application::Get().GetThreadPool().PushJob<void>([singleThreadPass, threadPass]() {
				
				TimeMeasure timer;
				double timestep = 0.0f;
				while (singleThreadPass->Play)
				{
					timestep += timer.Begin();
					// If timestep after conversion to seconds is not zero perform step
					if (timestep > 1000.0 * FLT_MIN)
					{
						timestep *= 0.001f;
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
						threadPass->AttemptSwap();
						timestep = 0.0;
					}
			
					timer.End();
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