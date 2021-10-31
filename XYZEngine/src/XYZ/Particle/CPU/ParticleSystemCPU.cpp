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
		m_ModuleThreadPass(other.m_ModuleThreadPass),
		m_EmitThreadPass(other.m_EmitThreadPass),
		m_RenderThreadPass(other.m_RenderThreadPass),
		m_MaxParticles(other.m_MaxParticles),
		m_Play(other.m_Play),
		m_Speed(other.m_Speed)
	{
	}

	ParticleSystemCPU::ParticleSystemCPU(ParticleSystemCPU&& other) noexcept
		:
		m_ParticleData(std::move(other.m_ParticleData)),
		m_ModuleThreadPass(std::move(other.m_ModuleThreadPass)),
		m_EmitThreadPass(std::move(other.m_EmitThreadPass)),
		m_RenderThreadPass(std::move(other.m_RenderThreadPass)),
		m_MaxParticles(other.m_MaxParticles),
		m_Play(other.m_Play),
		m_Speed(other.m_Speed)
	{
	}

	ParticleSystemCPU& ParticleSystemCPU::operator=(const ParticleSystemCPU& other)
	{
		m_ParticleData = other.m_ParticleData;
		m_ModuleThreadPass = other.m_ModuleThreadPass;
		m_EmitThreadPass = other.m_EmitThreadPass;
		m_RenderThreadPass = other.m_RenderThreadPass;
		m_MaxParticles = other.m_MaxParticles;
		m_Play = other.m_Play;
		m_Speed = other.m_Speed;
		return *this;
	}

	ParticleSystemCPU& ParticleSystemCPU::operator=(ParticleSystemCPU&& other) noexcept
	{
		m_ParticleData = std::move(other.m_ParticleData);
		m_ModuleThreadPass = std::move(other.m_ModuleThreadPass);
		m_EmitThreadPass = std::move(other.m_EmitThreadPass);
		m_RenderThreadPass = std::move(other.m_RenderThreadPass);
		m_MaxParticles = other.m_MaxParticles;
		m_Play = other.m_Play;
		m_Speed = other.m_Speed;
		return *this;
	}

	void ParticleSystemCPU::SetPhysicsWorld(b2World* world)
	{
		GetModuleData()->m_CollisionModule.SetPhysicsWorld(world);
	}

	void ParticleSystemCPU::Update(Timestep ts)
	{	
		XYZ_PROFILE_FUNC("ParticleSystemCPU::Update");
		if (m_Play)
		{		
			updatePhysics();
			particleThreadUpdate(ts.GetSeconds() * m_Speed);
		}
	}

	void ParticleSystemCPU::SetupForRender(Ref<SceneRenderer> renderer)
	{
		XYZ_PROFILE_FUNC("ParticleSystemCPU::SubmitLights");
		auto moduleData = m_ModuleThreadPass.GetRead<ModuleData>();
		const auto& lightModule = moduleData->m_LightModule;
		if (lightModule.m_TransformEntity.IsValid() 
		 && lightModule.m_LightEntity.IsValid()
		 && lightModule.m_LightEntity.HasComponent<PointLight2D>())
		{
			if (lightModule.IsEnabled())
			{
				const std::vector<glm::vec3>& lights	= lightModule.GetLights();
				const PointLight2D&			  light		= lightModule.m_LightEntity.GetComponent<PointLight2D>();
				const TransformComponent&     transform = lightModule.m_TransformEntity.GetComponent<TransformComponent>();
				for (const glm::vec3& l : lights)
					renderer->SubmitLight(light, transform.WorldTransform * glm::translate(l));
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
		m_ModuleThreadPass.Get<ModuleData>()->m_CollisionModule.SetMaxParticles(maxParticles);
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


	ScopedLock<ParticleSystemCPU::ModuleData> ParticleSystemCPU::GetModuleData()
	{
		return m_ModuleThreadPass.Get<ModuleData>();
	}

	ScopedLockRead<ParticleSystemCPU::ModuleData> ParticleSystemCPU::GetModuleDataRead() const
	{
		return m_ModuleThreadPass.GetRead<ModuleData>();
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
		ScopedLock<ModuleData> data = m_ModuleThreadPass.Get<ModuleData>();
		data->m_MainModule.UpdateParticles(timestep, particles);
		data->m_LightModule.UpdateParticles(timestep, particles);
		data->m_TextureAnimModule.UpdateParticles(timestep, particles);
		data->m_RotationOverLife.UpdateParticles(timestep, particles);
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
				particle.Position,
				particles.m_Size[i],
				particles.m_Rotation[i],
				particles.m_TexOffset[i]
			};
		}
		val->m_InstanceCount = endId;
	}
	void ParticleSystemCPU::updatePhysics()
	{
		// TODO: once physics world is thread safe, it can be in update function
		XYZ_PROFILE_FUNC("ParticleSystemCPU::updatePhysics");
		auto moduleData = GetModuleData();
		if (moduleData->m_CollisionModule.IsEnabled())
		{
			auto [startId, endId] = GetEmitter()->m_EmittedIDs;
			auto particleData	  = GetParticleData();
			moduleData->m_CollisionModule.Generate(particleData.As(), startId, endId);
			moduleData->m_CollisionModule.UpdateParticles(particleData.As());
		}
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
	ParticleSystemCPU::ModuleData::ModuleData()
	{
	}
}