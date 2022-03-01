#include "stdafx.h"
#include "ParticleSystem.h"
#include "XYZ/Renderer/SceneRenderer.h"

#include "XYZ/Scene/Components.h"
#include "XYZ/Core/Application.h"

#include "XYZ/Debug/Profiler.h"

namespace XYZ {
	static float CalcRatio(float length, float value)
	{
		return (length - value) / length;
	}
	ParticleSystem::ParticleSystem(uint32_t maxParticles)
		:
		m_MaxParticles(maxParticles),
		AnimationTiles(1, 1),
		AnimationStartFrame(0),
		AnimationCycleLength(0.0f),
		RotationEulerAngles(0.0f),
		RotationCycleLength(0.0f),
		Play(true),
		Speed(1.0f),
		m_Pool(maxParticles)
	{
		{
			ScopedLock<RenderData> write = m_RenderThreadPass.Write();
			write->ParticleData.resize(maxParticles);
		}
		{
			ScopedLock<RenderData> read = m_RenderThreadPass.Read();
			read->ParticleData.resize(maxParticles);
		}
	}

	ParticleSystem::~ParticleSystem()
	{
	}

	ParticleSystem::ParticleSystem(const ParticleSystem& other)
		:
		Emitter(other.Emitter),
		AnimationTiles(other.AnimationTiles),
		AnimationStartFrame(other.AnimationStartFrame),
		AnimationCycleLength(other.AnimationCycleLength),
		RotationEulerAngles(other.RotationEulerAngles),
		RotationCycleLength(other.RotationCycleLength),
		Play(other.Play),
		Speed(other.Speed),
		m_Pool(other.m_Pool),
		m_RenderThreadPass(other.m_RenderThreadPass),
		m_MaxParticles(other.m_MaxParticles)
	{
	}

	ParticleSystem::ParticleSystem(ParticleSystem&& other) noexcept
		:
		Emitter(other.Emitter),
		AnimationTiles(other.AnimationTiles),
		AnimationStartFrame(other.AnimationStartFrame),
		AnimationCycleLength(other.AnimationCycleLength),
		RotationEulerAngles(other.RotationEulerAngles),
		RotationCycleLength(other.RotationCycleLength),
		Play(other.Play),
		Speed(other.Speed),
		m_Pool(std::move(other.m_Pool)),
		m_RenderThreadPass(std::move(other.m_RenderThreadPass)),
		m_MaxParticles(other.m_MaxParticles)
	{
	}

	ParticleSystem& ParticleSystem::operator=(const ParticleSystem& other)
	{	
		Emitter = other.Emitter;
		AnimationTiles = other.AnimationTiles;
		AnimationStartFrame = other.AnimationStartFrame;
		RotationEulerAngles = other.RotationEulerAngles;
		RotationCycleLength = other.RotationCycleLength;
		Play = other.Play;
		Speed = other.Speed;

		m_Pool = other.m_Pool;
		m_RenderThreadPass = other.m_RenderThreadPass;
		m_MaxParticles = other.m_MaxParticles;

		return *this;
	}

	ParticleSystem& ParticleSystem::operator=(ParticleSystem&& other) noexcept
	{
		Emitter = other.Emitter;
		AnimationTiles = other.AnimationTiles;
		AnimationStartFrame = other.AnimationStartFrame;
		RotationEulerAngles = other.RotationEulerAngles;
		RotationCycleLength = other.RotationCycleLength;
		Play = other.Play;
		Speed = other.Speed;

		m_Pool = other.m_Pool;
		m_RenderThreadPass = std::move(other.m_RenderThreadPass);
		m_MaxParticles = other.m_MaxParticles;
		return *this;
	}

	void ParticleSystem::Update(Timestep ts)
	{	
		XYZ_PROFILE_FUNC("ParticleSystem::Update");
		if (Play)
		{		
			particleThreadUpdate(ts.GetSeconds() * Speed);
		}
	}

	
	
	void ParticleSystem::Reset()
	{
		for (uint32_t i = 0; i < m_Pool.GetAliveParticles(); ++i)
			m_Pool.Kill(i);
	}

	void ParticleSystem::SetMaxParticles(uint32_t maxParticles)
	{
		m_MaxParticles = maxParticles;
		m_Pool.SetMaxParticles(maxParticles);
		{
			m_RenderThreadPass.Read()->ParticleData.resize(maxParticles);
		}
		m_RenderThreadPass.Swap();
		{
			m_RenderThreadPass.Read()->ParticleData.resize(maxParticles);
		}
	}

	uint32_t ParticleSystem::GetMaxParticles() const
	{
		return m_MaxParticles;
	}

	uint32_t ParticleSystem::GetAliveParticles() const
	{
		return m_Pool.GetAliveParticles();
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
		float speed = Speed;
		if (m_MaxParticles == 0)
			return;

		Application::Get().GetThreadPool().PushJob<void>([this, timestep, speed]() {
			XYZ_PROFILE_FUNC("ParticleSystem::particleThreadUpdate Job");
			{			
				Emitter.Emit(timestep, m_Pool);
		
				update(timestep);
				buildRenderData();
			}
			m_RenderThreadPass.Swap();	
		});
	}
	void ParticleSystem::update(Timestep timestep)
	{
		XYZ_PROFILE_FUNC("ParticleSystem::update");
		auto particles = m_Pool.Particles;


		const uint32_t stageCount = AnimationTiles.x * AnimationTiles.y;
		const float columnSize    = 1.0f / AnimationTiles.x;
		const float rowSize		  = 1.0f / AnimationTiles.y;

		uint32_t aliveParticles = m_Pool.GetAliveParticles();
		for (uint32_t i = 0; i < aliveParticles; ++i)
		{
			particles[i].Position += particles[i].Velocity * timestep.GetSeconds();
			particles[i].LifeRemaining -= timestep.GetSeconds();		
		
			const float ratio = CalcRatio(AnimationCycleLength, particles[i].LifeRemaining);
			const float stageProgress = ratio * stageCount;
		
			const uint32_t index = (uint32_t)floor(stageProgress);
			const float column = index % AnimationTiles.x;
			const float row = index / AnimationTiles.y;

			particles[i].TexOffset = glm::vec2(column / (float)AnimationTiles.x, row / (float)AnimationTiles.y);
		}

		
		//data.TextureAnimationUpdater.UpdateParticles(timestep, data.Pool);
		//data.RotationOverLifeUpdater.UpdateParticles(timestep, data.Pool);

		for (uint32_t i = 0; i < aliveParticles; ++i)
		{
			if (particles[i].LifeRemaining <= 0.0f)
			{
				aliveParticles--;
				m_Pool.Kill(i);
				if (Emitter.m_AliveLights != 0)
					Emitter.m_AliveLights--;
			}
		}
	}
	void ParticleSystem::buildRenderData()
	{
		XYZ_PROFILE_FUNC("ParticleSystem::buildRenderData");
		ScopedLock<RenderData> val = m_RenderThreadPass.Write();
		
		const uint32_t endId = m_Pool.GetAliveParticles();
		for (uint32_t i = 0; i < endId; ++i)
		{
			const auto& particle = m_Pool.Particles[i];
			val->ParticleData[i] = ParticleRenderData{
				particle.Color,
				particle.Position,
				particle.Size,
				particle.Rotation,
				particle.TexOffset
			};
		}
		val->LightData.clear();
		val->LightData.reserve(Emitter.MaxLights);
		for (uint32_t i = 0; i < endId && i < Emitter.MaxLights; ++i)
		{
			const auto& particle = m_Pool.Particles[i];
			auto& light = val->LightData.emplace_back();
			light.Color = particle.LightColor;
			light.Position = particle.Position;
			light.Radius = particle.LightRadius;
			light.Intensity = particle.LightIntensity;
		}
		val->ParticleCount = endId;
	}

	ParticleSystem::RenderData::RenderData(uint32_t maxParticles)
	{
		ParticleData.resize(maxParticles);
	}
}