#include "stdafx.h"
#include "ParticleSystemCPU.h"
#include "XYZ/Renderer/SceneRenderer.h"

#include "XYZ/Scene/Components.h"
#include "XYZ/Core/Application.h"

#include "XYZ/Debug/Profiler.h"

namespace XYZ {
	ParticleSystemCPU::ParticleSystemCPU()
		:
		m_ModuleThreadPass(0),
		m_MaxParticles(0),
		m_Play(true),
		m_Speed(1.0f)
	{
	}
	ParticleSystemCPU::ParticleSystemCPU(uint32_t maxParticles)
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

	ParticleSystemCPU::~ParticleSystemCPU()
	{
	}

	ParticleSystemCPU::ParticleSystemCPU(const ParticleSystemCPU& other)
		:
		m_ModuleThreadPass(other.m_ModuleThreadPass),
		m_RenderThreadPass(other.m_RenderThreadPass),
		m_MaxParticles(other.m_MaxParticles),
		m_Play(other.m_Play),
		m_Speed(other.m_Speed)
	{
	}

	ParticleSystemCPU::ParticleSystemCPU(ParticleSystemCPU&& other) noexcept
		:
		m_ModuleThreadPass(std::move(other.m_ModuleThreadPass)),
		m_RenderThreadPass(std::move(other.m_RenderThreadPass)),
		m_MaxParticles(other.m_MaxParticles),
		m_Play(other.m_Play),
		m_Speed(other.m_Speed)
	{
	}

	ParticleSystemCPU& ParticleSystemCPU::operator=(const ParticleSystemCPU& other)
	{
		m_ModuleThreadPass = other.m_ModuleThreadPass;
		m_RenderThreadPass = other.m_RenderThreadPass;
		m_MaxParticles = other.m_MaxParticles;
		m_Play = other.m_Play;
		m_Speed = other.m_Speed;
		return *this;
	}

	ParticleSystemCPU& ParticleSystemCPU::operator=(ParticleSystemCPU&& other) noexcept
	{
		m_ModuleThreadPass = std::move(other.m_ModuleThreadPass);
		m_RenderThreadPass = std::move(other.m_RenderThreadPass);
		m_MaxParticles = other.m_MaxParticles;
		m_Play = other.m_Play;
		m_Speed = other.m_Speed;
		return *this;
	}

	void ParticleSystemCPU::Update(Timestep ts)
	{	
		XYZ_PROFILE_FUNC("ParticleSystemCPU::Update");
		if (m_Play)
		{		
			particleThreadUpdate(ts.GetSeconds() * m_Speed);
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

	void ParticleSystemCPU::Reset()
	{
		auto moduleData = GetModuleData();
		for (uint32_t i = 0; i < moduleData->Particles.GetAliveParticles(); ++i)
			moduleData->Particles.Kill(i);
		
		moduleData->Light.Reset();
	}

	void ParticleSystemCPU::SetMaxParticles(uint32_t maxParticles)
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

	void ParticleSystemCPU::SetSpeed(float speed)
	{
		m_Speed = speed;
	}

	uint32_t ParticleSystemCPU::GetMaxParticles() const
	{
		return m_MaxParticles;
	}

	uint32_t ParticleSystemCPU::GetAliveParticles() const
	{
		return GetModuleDataRead()->Particles.GetAliveParticles();
	}

	float ParticleSystemCPU::GetSpeed() const
	{
		return m_Speed;
	}

	ScopedLock<ParticleSystemCPU::ModuleData> ParticleSystemCPU::GetModuleData()
	{
		return m_ModuleThreadPass.Get<ModuleData>();
	}

	ScopedLockRead<ParticleSystemCPU::ModuleData> ParticleSystemCPU::GetModuleDataRead() const
	{
		return m_ModuleThreadPass.GetRead<ModuleData>();
	}

	ScopedLock<ParticleSystemCPU::RenderData> ParticleSystemCPU::GetRenderData()
	{
		return m_RenderThreadPass.Read();
	}

	ScopedLockRead<ParticleSystemCPU::RenderData> ParticleSystemCPU::GetRenderDataRead() const
	{
		return m_RenderThreadPass.ReadRead();
	}

	void ParticleSystemCPU::particleThreadUpdate(float timestep)
	{
		float speed = m_Speed;

		Application::Get().GetThreadPool().PushJob<void>([this, timestep, speed]() {
			XYZ_PROFILE_FUNC("ParticleSystemCPU::particleThreadUpdate Job");
			{			
				ScopedLock<ModuleData> moduleData = m_ModuleThreadPass.Get<ModuleData>();
				emit(timestep, moduleData.As(), speed);
				update(timestep, moduleData.As());
				buildRenderData(moduleData.As());
			}
			m_RenderThreadPass.AttemptSwap();		
		});
	}
	void ParticleSystemCPU::update(Timestep timestep, ModuleData& data)
	{
		XYZ_PROFILE_FUNC("ParticleSystemCPU::update");
		data.Main.UpdateParticles(timestep, data.Particles);
		const std::vector<uint32_t>& killed = data.Main.Killed();

		data.Light.UpdateParticles(timestep, data.Particles);
		data.TextureAnim.UpdateParticles(timestep, data.Particles);
		data.RotationOverLife.UpdateParticles(timestep, data.Particles);

		for (const uint32_t i : killed)
			data.Particles.Kill(i);
	}
	void ParticleSystemCPU::emit(Timestep timestep, ModuleData& data, float speed)
	{
		XYZ_PROFILE_FUNC("ParticleSystemCPU::emit");
		data.Emitter.Emit(timestep, data.Particles);	
	}
	void ParticleSystemCPU::buildRenderData(ModuleData& data)
	{
		XYZ_PROFILE_FUNC("ParticleSystemCPU::buildRenderData");
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

	ParticleSystemCPU::RenderData::RenderData(uint32_t maxParticles)
	{
		Data.resize(maxParticles);
	}
	ParticleSystemCPU::ModuleData::ModuleData(uint32_t maxParticles)
		:
		Particles(maxParticles)
	{
	}
}