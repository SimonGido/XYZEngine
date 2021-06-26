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
	}

	void ParticleSystemCPU::Update(Timestep ts)
	{	
		if (m_Play)
		{
			particleThreadUpdate(ts.GetSeconds());
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
	}

	void ParticleSystemCPU::Play()
	{
		m_Play = true;
	}
	void ParticleSystemCPU::Stop()
	{
		m_Play = false;
	}
	void ParticleSystemCPU::AddEmitter(const Ref<ParticleEmitterCPU>& emitter)
	{
		std::scoped_lock lock(m_SingleThreadPass->Mutex);
		m_SingleThreadPass->Emitters.push_back(emitter);
	}

	void ParticleSystemCPU::AddUpdater(const Ref<ParticleUpdater>& updater)
	{
		std::scoped_lock lock(m_SingleThreadPass->Mutex);
		m_SingleThreadPass->Updaters.push_back(updater);
	}
	void ParticleSystemCPU::RemoveEmitter(const Ref<ParticleEmitterCPU>& emitter)
	{
		std::scoped_lock lock(m_SingleThreadPass->Mutex);
		auto& emitters = m_SingleThreadPass->Emitters;
		for (size_t i = 0; i < emitters.size(); ++i)
		{
			if (emitters[i].Raw() == emitter.Raw())
			{
				emitters.erase(emitters.begin() + i);
				return;
			}
		}
	}
	void ParticleSystemCPU::RemoveUpdater(const Ref<ParticleUpdater>& updater)
	{
		std::scoped_lock lock(m_SingleThreadPass->Mutex);
		auto& updaters = m_SingleThreadPass->Updaters;
		for (size_t i = 0; i < updaters.size(); ++i)
		{
			if (updaters[i].Raw() == updater.Raw())
			{
				updaters.erase(updaters.begin() + i);
				return;
			}
		}
	}
	std::vector<Ref<ParticleUpdater>> ParticleSystemCPU::GetUpdaters() const
	{
		std::scoped_lock lock(m_SingleThreadPass->Mutex);
		return m_SingleThreadPass->Updaters;
	}
	std::vector<Ref<ParticleEmitterCPU>> ParticleSystemCPU::GetEmitters() const
	{
		std::scoped_lock lock(m_SingleThreadPass->Mutex);
		return m_SingleThreadPass->Emitters;
	}
	void ParticleSystemCPU::particleThreadUpdate(float timestep)
	{
		auto singleThreadPass = m_SingleThreadPass;
		auto threadPass = m_ThreadPass;
		Application::Get().GetThreadPool().PushJob<void>([singleThreadPass, threadPass, timestep]() {			
			{
				std::scoped_lock lock(singleThreadPass->Mutex);
				ScopedLockReference<DoubleThreadPass> val = threadPass->Write();

				for (auto& emitter : singleThreadPass->Emitters)
					emitter->Emit(timestep, &singleThreadPass->Particles);

				for (auto& updater : singleThreadPass->Updaters)
					updater->UpdateParticles(timestep, &singleThreadPass->Particles);

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
		});
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
		Particles(maxParticles)
	{
	}
}