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
			write->RenderParticleData.resize(maxParticles);
		}
		{
			ScopedLock<RenderData> read = m_RenderThreadPass.Read();
			read->RenderParticleData.resize(maxParticles);
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
				
				m_Renderer->InstanceCount = val->InstanceCount;
				m_Renderer->InstanceVBO->Update(val->RenderParticleData.data(), m_Renderer->InstanceCount * sizeof(ParticleRenderData));
			}
		}
	}

	void ParticleSystemCPU::SubmitLights(Ref<SceneRenderer> renderer)
	{
		XYZ_PROFILE_FUNC("ParticleSystemCPU::SubmitLights");
		auto updateData = m_UpdateThreadPass.GetRead<UpdateData>();
		const auto& lightUpdater = updateData->LightUpdater;
		const SceneEntity& lightEntity = lightUpdater.GetLightEntity();
		const SceneEntity& transformEntity = lightUpdater.GetTransformEntity();
		if (transformEntity.IsValid() 
		 && lightEntity.IsValid()
		 && lightEntity.HasComponent<PointLight2D>())
		{
			auto particleData = m_ParticleData.GetRead<ParticleDataBuffer>();
			if (lightUpdater.IsEnabled())
			{
				const PointLight2D& light = lightEntity.GetComponent<PointLight2D>();
				const TransformComponent& transform = transformEntity.GetComponent<TransformComponent>();
				uint32_t count = std::min(lightUpdater.GetMaxLights(), particleData->GetAliveParticles());
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

	ScopedLock<ParticleSystemCPU::EmitData> ParticleSystemCPU::GetEmitData()
	{
		return m_EmitThreadPass.Get<EmitData>();
	}

	ScopedLockRead<ParticleSystemCPU::EmitData> ParticleSystemCPU::GetEmitDataRead() const
	{
		return m_EmitThreadPass.GetRead<EmitData>();
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
		if (data->TimeUpdater.IsEnabled())
			data->TimeUpdater.UpdateParticles(timestep, &particles);
		if (data->PositionUpdater.IsEnabled())
			data->PositionUpdater.UpdateParticles(timestep, &particles);
		if (data->LightUpdater.IsEnabled())
			data->LightUpdater.UpdateParticles(timestep, &particles);
	}
	void ParticleSystemCPU::emit(Timestep timestep, ParticleDataBuffer& particles)
	{
		XYZ_PROFILE_FUNC("ParticleSystemCPU::emit");
		ScopedLock<EmitData> data = m_EmitThreadPass.Get<EmitData>();
		data->EmittedParticles += timestep * data->EmitRate;

		const uint32_t newParticles = (uint32_t)data->EmittedParticles;
		const uint32_t startId =particles.GetAliveParticles();
		const uint32_t endId = std::min(startId + newParticles, particles.GetMaxParticles() - 1);
		if (newParticles)
			data->EmittedParticles = 0.0f;

		if (data->ShapeGenerator.IsEnabled())
			data->ShapeGenerator.Generate(&particles, startId, endId);
		if (data->LifeGenerator.IsEnabled())
			data->LifeGenerator.Generate(&particles, startId, endId);
		if (data->RandomVelGenerator.IsEnabled())
			data->RandomVelGenerator.Generate(&particles, startId, endId);
		
		for (uint32_t i = startId; i < endId; ++i)
			particles.Wake(i);
	}
	void ParticleSystemCPU::buildRenderData(ParticleDataBuffer& particles)
	{
		XYZ_PROFILE_FUNC("ParticleSystemCPU::buildRenderData");
		ScopedLock<RenderData> val = m_RenderThreadPass.Write();
		uint32_t endId = particles.GetAliveParticles();
		for (uint32_t i = 0; i < endId; ++i)
		{
			auto& particle = particles.m_Particle[i];
			val->RenderParticleData[i] = ParticleRenderData{
				particle.Color,
				particles.m_TexCoord[i],
				glm::vec2(particle.Position.x, particle.Position.y),
				particles.m_Size[i],
				particles.m_Rotation[i]
			};
		}
		val->InstanceCount = endId;
	}
	ParticleSystemCPU::RenderData::RenderData()
		:
		InstanceCount(0)
	{
	}
	ParticleSystemCPU::RenderData::RenderData(uint32_t maxParticles)
		:
		InstanceCount(0)
	{
		RenderParticleData.resize(maxParticles);
	}
}