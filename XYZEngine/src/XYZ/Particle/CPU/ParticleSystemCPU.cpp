#include "stdafx.h"
#include "ParticleSystemCPU.h"
#include "XYZ/Renderer/SceneRenderer.h"

#include "XYZ/Scene/Components.h"
#include "XYZ/Core/Application.h"

#include "XYZ/Debug/Profiler.h"

namespace XYZ {

	ParticleSystemCPU::ParticleSystemCPU(uint32_t maxParticles)
		:
		m_ParticleData(maxParticles)
	{
		m_Renderer = Ref<ParticleRendererCPU>::Create(maxParticles);
		{
			ScopedLock<RenderData> write = m_RenderThreadPass.Write();
			write->m_RenderParticleData.resize(maxParticles);
		}
		{
			ScopedLock<RenderData> read = m_RenderThreadPass.Read();
			read->m_RenderParticleData.resize(maxParticles);
		}
	}

	ParticleSystemCPU::~ParticleSystemCPU()
	{
	}

	void ParticleSystemCPU::Update(Timestep ts)
	{	
		XYZ_PROFILE_FUNC("ParticleSystemCPU::Update");
		if (m_Play)
		{		
			particleThreadUpdate(ts.GetSeconds());
			{
				ScopedLock<RenderData> val = m_RenderThreadPass.Read();
				
				m_Renderer->m_InstanceCount = val->m_InstanceCount;
				m_Renderer->m_InstanceVBO->Update(val->m_RenderParticleData.data(), m_Renderer->m_InstanceCount * sizeof(ParticleRenderData));
			}
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

	ScopedLock<ParticleEmitterCPU> ParticleSystemCPU::GetEmitData()
	{
		return m_EmitThreadPass.Get<ParticleEmitterCPU>();
	}

	ScopedLockRead<ParticleEmitterCPU> ParticleSystemCPU::GetEmitDataRead() const
	{
		return m_EmitThreadPass.GetRead<ParticleEmitterCPU>();
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
		data->m_TimeUpdater.UpdateParticles(timestep, &particles);
		data->m_PositionUpdater.UpdateParticles(timestep, &particles);
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
		uint32_t endId = particles.GetAliveParticles();
		for (uint32_t i = 0; i < endId; ++i)
		{
			auto& particle = particles.m_Particle[i];
			val->m_RenderParticleData[i] = ParticleRenderData{
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
		m_RenderParticleData.resize(maxParticles);
	}
}