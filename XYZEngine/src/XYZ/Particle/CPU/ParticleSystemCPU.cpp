#include "stdafx.h"
#include "ParticleSystemCPU.h"
#include "XYZ/Renderer/SceneRenderer.h"

#include "XYZ/Scene/Components.h"
#include "XYZ/Core/Application.h"

#include "XYZ/Debug/Profiler.h"

namespace XYZ {
	ParticleSystemCPU::ParticleSystemCPU()
		:
		m_ParticleData(0),
		m_MaxParticles(0),
		m_Play(false),
		m_Speed(1.0f)
	{
		{
			ScopedLock<RenderData> write = m_RenderThreadPass.Write();
			write->m_RenderParticleData.SetElementSize(sizeof(ParticleRenderData));
		}
		{
			ScopedLock<RenderData> read = m_RenderThreadPass.Read();
			read->m_RenderParticleData.SetElementSize(sizeof(ParticleRenderData));
		}
	}
	ParticleSystemCPU::ParticleSystemCPU(uint32_t maxParticles)
		:
		m_ParticleData(maxParticles),
		m_MaxParticles(maxParticles),
		m_Play(false),
		m_Speed(1.0f)
	{
		{
			ScopedLock<RenderData> write = m_RenderThreadPass.Write();
			write->m_RenderParticleData.SetElementSize(sizeof(ParticleRenderData));
			write->m_RenderParticleData.Resize(maxParticles);
		}
		{
			ScopedLock<RenderData> read = m_RenderThreadPass.Read();
			read->m_RenderParticleData.SetElementSize(sizeof(ParticleRenderData));
			read->m_RenderParticleData.Resize(maxParticles);
		}
	}

	ParticleSystemCPU::~ParticleSystemCPU()
	{
	}

	ParticleSystemCPU::ParticleSystemCPU(const ParticleSystemCPU& other)
		:
		m_ParticleData(other.m_ParticleData),
		m_UpdateThreadPass(other.m_UpdateThreadPass),
		m_EmitThreadPass(other.m_EmitThreadPass),
		m_RenderThreadPass(other.m_RenderThreadPass),
		m_LightPass(other.m_LightPass),
		m_MaxParticles(other.m_MaxParticles),
		m_Play(other.m_Play),
		m_Speed(other.m_Speed)
	{
	}

	ParticleSystemCPU::ParticleSystemCPU(ParticleSystemCPU&& other) noexcept
		:
		m_ParticleData(std::move(other.m_ParticleData)),
		m_UpdateThreadPass(std::move(other.m_UpdateThreadPass)),
		m_EmitThreadPass(std::move(other.m_EmitThreadPass)),
		m_RenderThreadPass(std::move(other.m_RenderThreadPass)),
		m_LightPass(std::move(other.m_LightPass)),
		m_MaxParticles(other.m_MaxParticles),
		m_Play(other.m_Play),
		m_Speed(other.m_Speed)
	{
	}

	ParticleSystemCPU& ParticleSystemCPU::operator=(const ParticleSystemCPU& other)
	{
		m_ParticleData = other.m_ParticleData;
		m_UpdateThreadPass = other.m_UpdateThreadPass;
		m_EmitThreadPass = other.m_EmitThreadPass;
		m_RenderThreadPass = other.m_RenderThreadPass;
		m_LightPass = other.m_LightPass;
		m_MaxParticles = other.m_MaxParticles;
		m_Play = other.m_Play;
		m_Speed = other.m_Speed;
		return *this;
	}

	ParticleSystemCPU& ParticleSystemCPU::operator=(ParticleSystemCPU&& other) noexcept
	{
		m_ParticleData = std::move(other.m_ParticleData);
		m_UpdateThreadPass = std::move(other.m_UpdateThreadPass);
		m_EmitThreadPass = std::move(other.m_EmitThreadPass);
		m_RenderThreadPass = std::move(other.m_RenderThreadPass);
		m_LightPass = std::move(other.m_LightPass);
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

	void ParticleSystemCPU::SubmitLights(Ref<SceneRenderer> renderer)
	{
		XYZ_PROFILE_FUNC("ParticleSystemCPU::SubmitLights");
		auto updateData = m_UpdateThreadPass.GetRead<UpdateData>();
		const auto& lightUpdater = updateData->m_LightUpdater;
		if (lightUpdater.m_TransformEntity.IsValid() 
		 && lightUpdater.m_LightEntity.IsValid()
		 && lightUpdater.m_LightEntity.HasComponent<PointLight2D>())
		{
			auto particleData = m_ParticleData.GetRead<ParticleDataBuffer>();
			if (lightUpdater.m_Enabled)
			{
				const PointLight2D& light = lightUpdater.m_LightEntity.GetComponent<PointLight2D>();
				const TransformComponent& transform = lightUpdater.m_TransformEntity.GetComponent<TransformComponent>();
				uint32_t count = std::min(lightUpdater.m_MaxLights, particleData->GetAliveParticles());
				for (uint32_t i = 0; i < count; ++i)
					renderer->SubmitLight(light, transform.WorldTransform * glm::translate(particleData->m_Lights[i]));
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

	void ParticleSystemCPU::SetMaxParticles(uint32_t maxParticles)
	{
		m_MaxParticles = maxParticles;
		m_ParticleData.Get<ParticleDataBuffer>()->SetMaxParticles(maxParticles);
		{
			m_RenderThreadPass.Read()->m_RenderParticleData.Resize(maxParticles);
		}
		m_RenderThreadPass.Swap();
		{
			m_RenderThreadPass.Read()->m_RenderParticleData.Resize(maxParticles);
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
		return GetParticleDataRead()->GetAliveParticles();
	}

	float ParticleSystemCPU::GetSpeed() const
	{
		return m_Speed;
	}


	ScopedLock<ParticleDataBuffer> ParticleSystemCPU::GetParticleData()
	{
		return m_ParticleData.Get<ParticleDataBuffer>();
	}

	ScopedLockRead<ParticleDataBuffer> ParticleSystemCPU::GetParticleDataRead() const
	{
		return m_ParticleData.GetRead<ParticleDataBuffer>();
	}


	ScopedLock<ParticleSystemCPU::UpdateData> ParticleSystemCPU::GetUpdateData()
	{
		return m_UpdateThreadPass.Get<UpdateData>();
	}

	ScopedLockRead<ParticleSystemCPU::UpdateData> ParticleSystemCPU::GetUpdateDataRead() const
	{
		return m_UpdateThreadPass.GetRead<UpdateData>();
	}

	ScopedLock<ParticleEmitterCPU> ParticleSystemCPU::GetEmitter()
	{
		return m_EmitThreadPass.Get<ParticleEmitterCPU>();
	}

	ScopedLockRead<ParticleEmitterCPU> ParticleSystemCPU::GetEmitterRead() const
	{
		return m_EmitThreadPass.GetRead<ParticleEmitterCPU>();
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
		Application::Get().GetThreadPool().PushJob<void>([this, timestep]() {
			XYZ_PROFILE_FUNC("ParticleSystemCPU::particleThreadUpdate Job");
			{
				ScopedLock<ParticleDataBuffer> particleData = m_ParticleData.Get<ParticleDataBuffer>();
				update(timestep, particleData.As());
				emit(timestep, particleData.As());
				buildRenderData(particleData.As());
			}
			m_RenderThreadPass.AttemptSwap();		
		});
	}
	void ParticleSystemCPU::update(Timestep timestep, ParticleDataBuffer& particles)
	{
		XYZ_PROFILE_FUNC("ParticleSystemCPU::update");
		ScopedLockRead<UpdateData> data = m_UpdateThreadPass.GetRead<UpdateData>();
		data->m_MainUpdater.UpdateParticles(timestep, &particles);
		data->m_LightUpdater.UpdateParticles(timestep, &particles);
	}
	void ParticleSystemCPU::emit(Timestep timestep, ParticleDataBuffer& particles)
	{
		XYZ_PROFILE_FUNC("ParticleSystemCPU::emit");
		ScopedLock<ParticleEmitterCPU> data = m_EmitThreadPass.Get<ParticleEmitterCPU>();
		data->Emit(timestep, particles);
	}
	void ParticleSystemCPU::buildRenderData(ParticleDataBuffer& particles)
	{
		XYZ_PROFILE_FUNC("ParticleSystemCPU::buildRenderData");
		ScopedLock<RenderData> val = m_RenderThreadPass.Write();
		ParticleRenderData* buffer = val->m_RenderParticleData.As<ParticleRenderData>();
		uint32_t endId = particles.GetAliveParticles();
		for (uint32_t i = 0; i < endId; ++i)
		{
			auto& particle = particles.m_Particle[i];
			buffer[i] = ParticleRenderData{
				particle.Color,
				particles.m_TexCoord[i],
				glm::vec2(particle.Position.x, particle.Position.y),
				particles.m_Size[i],
				particles.m_Rotation[i]
			};
		}
		val->m_InstanceCount = endId;
	}
	ParticleSystemCPU::RenderData::RenderData()
		:
		m_InstanceCount(0)
	{
	}
	ParticleSystemCPU::RenderData::RenderData(uint32_t maxParticles)
		:
		m_InstanceCount(0)
	{
		m_RenderParticleData.Resize(maxParticles);
	}
	ParticleSystemCPU::UpdateData::UpdateData()
	{
	}
}