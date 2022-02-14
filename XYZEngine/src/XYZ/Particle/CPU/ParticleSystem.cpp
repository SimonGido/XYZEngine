#include "stdafx.h"
#include "ParticleSystem.h"
#include "XYZ/Renderer/SceneRenderer.h"

#include "XYZ/Scene/Components.h"
#include "XYZ/Core/Application.h"

#include "XYZ/Debug/Profiler.h"

namespace XYZ {
	ParticleSystem::ParticleSystem()
		:
		m_ModuleThreadPass(0),
		m_MaxParticles(0),
		m_Play(true),
		m_Speed(1.0f)
	{
	}
	ParticleSystem::ParticleSystem(uint32_t maxParticles)
		:
		m_ModuleThreadPass(maxParticles),
		m_MaxParticles(maxParticles),
		m_Play(true),
		m_Speed(1.0f)
	{
		{
			ScopedLock<RenderData> write = m_RenderThreadPass.Write();
			write->Data.resize(maxParticles);
		}
		{
			ScopedLock<RenderData> read = m_RenderThreadPass.Read();
			read->Data.resize(maxParticles);
		}
	}

	ParticleSystem::~ParticleSystem()
	{
	}

	ParticleSystem::ParticleSystem(const ParticleSystem& other)
		:
		m_ModuleThreadPass(other.m_ModuleThreadPass),
		m_RenderThreadPass(other.m_RenderThreadPass),
		m_MaxParticles(other.m_MaxParticles),
		m_Play(other.m_Play),
		m_Speed(other.m_Speed)
	{
	}

	ParticleSystem::ParticleSystem(ParticleSystem&& other) noexcept
		:
		m_ModuleThreadPass(std::move(other.m_ModuleThreadPass)),
		m_RenderThreadPass(std::move(other.m_RenderThreadPass)),
		m_MaxParticles(other.m_MaxParticles),
		m_Play(other.m_Play),
		m_Speed(other.m_Speed)
	{
	}

	ParticleSystem& ParticleSystem::operator=(const ParticleSystem& other)
	{
		m_ModuleThreadPass = other.m_ModuleThreadPass;
		m_RenderThreadPass = other.m_RenderThreadPass;
		m_MaxParticles = other.m_MaxParticles;
		m_Play = other.m_Play;
		m_Speed = other.m_Speed;
		return *this;
	}

	ParticleSystem& ParticleSystem::operator=(ParticleSystem&& other) noexcept
	{
		m_ModuleThreadPass = std::move(other.m_ModuleThreadPass);
		m_RenderThreadPass = std::move(other.m_RenderThreadPass);
		m_MaxParticles = other.m_MaxParticles;
		m_Play = other.m_Play;
		m_Speed = other.m_Speed;
		return *this;
	}

	void ParticleSystem::Update(Timestep ts)
	{	
		XYZ_PROFILE_FUNC("ParticleSystem::Update");
		if (m_Play)
		{		
			particleThreadUpdate(ts.GetSeconds() * m_Speed);
		}
	}

	
	void ParticleSystem::Play()
	{
		m_Play = true;
	}
	void ParticleSystem::Stop()
	{
		m_Play = false;
	}

	void ParticleSystem::Reset()
	{
		auto moduleData = GetModuleData();
		for (uint32_t i = 0; i < moduleData->Particles.GetAliveParticles(); ++i)
			moduleData->Particles.Kill(i);
		
		moduleData->Light.Reset();
	}

	void ParticleSystem::SetMaxParticles(uint32_t maxParticles)
	{
		m_MaxParticles = maxParticles;
		auto moduleData = GetModuleData();
		moduleData->Particles.SetMaxParticles(maxParticles);
		{
			m_RenderThreadPass.Read()->Data.resize(maxParticles);
		}
		m_RenderThreadPass.Swap();
		{
			m_RenderThreadPass.Read()->Data.resize(maxParticles);
		}
	}

	void ParticleSystem::SetSpeed(float speed)
	{
		m_Speed = speed;
	}

	uint32_t ParticleSystem::GetMaxParticles() const
	{
		return m_MaxParticles;
	}

	uint32_t ParticleSystem::GetAliveParticles() const
	{
		return GetModuleDataRead()->Particles.GetAliveParticles();
	}

	float ParticleSystem::GetSpeed() const
	{
		return m_Speed;
	}

	ScopedLock<ParticleSystem::ModuleData> ParticleSystem::GetModuleData()
	{
		return m_ModuleThreadPass.Get<ModuleData>();
	}

	ScopedLockRead<ParticleSystem::ModuleData> ParticleSystem::GetModuleDataRead() const
	{
		return m_ModuleThreadPass.GetRead<ModuleData>();
	}

	ScopedLock<ParticleSystem::RenderData> ParticleSystem::GetRenderData()
	{
		return m_RenderThreadPass.Read();
	}

	ScopedLockRead<ParticleSystem::RenderData> ParticleSystem::GetRenderDataRead() const
	{
		return m_RenderThreadPass.ReadRead();
	}

	void ParticleSystem::particleThreadUpdate(float timestep)
	{
		float speed = m_Speed;

		Application::Get().GetThreadPool().PushJob<void>([this, timestep, speed]() {
			XYZ_PROFILE_FUNC("ParticleSystem::particleThreadUpdate Job");
			{			
				ScopedLock<ModuleData> moduleData = m_ModuleThreadPass.Get<ModuleData>();
				auto& particles = moduleData->Particles;
				auto[startId, endId] = moduleData->Emitter.Emit(timestep, particles);

				moduleData->Main.Generate(particles, startId, endId);
				moduleData->Shape.Generate(particles, startId, endId);
				moduleData->Life.Generate(particles, startId, endId);
				moduleData->RandomVelocity.Generate(particles, startId, endId);

				update(timestep, moduleData.As());
				buildRenderData(moduleData.As());
			}
			m_RenderThreadPass.AttemptSwap();		
		});
	}
	void ParticleSystem::update(Timestep timestep, ModuleData& data)
	{
		XYZ_PROFILE_FUNC("ParticleSystem::update");
		auto particles = data.Particles.Particle;

		uint32_t aliveParticles = data.Particles.GetAliveParticles();
		for (uint32_t i = 0; i < aliveParticles; ++i)
		{
			particles[i].Position += particles[i].Velocity * timestep.GetSeconds();
			particles[i].LifeRemaining -= timestep.GetSeconds();
			
		}

		data.Light.UpdateParticles(timestep, data.Particles);
		data.TextureAnim.UpdateParticles(timestep, data.Particles);
		data.RotationOverLife.UpdateParticles(timestep, data.Particles);

		for (uint32_t i = 0; i < aliveParticles; ++i)
		{
			if (particles[i].LifeRemaining <= 0.0f)
			{
				aliveParticles--;
				data.Particles.Kill(i);
			}
		}
	}
	void ParticleSystem::buildRenderData(ModuleData& data)
	{
		XYZ_PROFILE_FUNC("ParticleSystem::buildRenderData");
		ScopedLock<RenderData> val = m_RenderThreadPass.Write();
		
		const uint32_t endId = data.Particles.GetAliveParticles();
		for (uint32_t i = 0; i < endId; ++i)
		{
			const auto& particle = data.Particles.Particle[i];
			val->Data[i] = ParticleRenderData{
				particle.Color,
				particle.Position,
				data.Particles.Size[i],
				data.Particles.Rotation[i],
				data.Particles.TexOffset[i]
			};
		}
		val->InstanceCount = endId;
	}

	ParticleSystem::RenderData::RenderData(uint32_t maxParticles)
	{
		Data.resize(maxParticles);
	}
	ParticleSystem::ModuleData::ModuleData(uint32_t maxParticles)
		:
		Particles(maxParticles)
	{
	}
}