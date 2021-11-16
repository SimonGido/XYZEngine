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
		m_ModuleThreadPass(maxParticles),
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

	void ParticleSystemCPU::SetSceneEntity(const SceneEntity& entity)
	{
		m_Entity = entity;
	}

	void ParticleSystemCPU::SetPhysicsWorld(PhysicsWorld2D* world)
	{
		GetModuleData()->m_PhysicsModule.SetPhysicsWorld(world);
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
				const std::vector<glm::vec3>& lights = lightModule.GetLights();
				const PointLight2D& light = lightModule.m_LightEntity.GetComponent<PointLight2D>();
				const TransformComponent& transform = lightModule.m_TransformEntity.GetComponent<TransformComponent>();
				for (const glm::vec3& l : lights)
					renderer->SubmitLight(light, transform.WorldTransform * glm::translate(l));
			}
		}
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
		for (uint32_t i = 0; i < moduleData->m_Particles.GetAliveParticles(); ++i)
			moduleData->m_Particles.Kill(i);
		
		moduleData->m_LightModule.Reset();
		moduleData->m_PhysicsModule.Reset();
	}

	void ParticleSystemCPU::SetMaxParticles(uint32_t maxParticles)
	{
		m_MaxParticles = maxParticles;
		auto moduleData = GetModuleData();
		moduleData->m_PhysicsModule.SetMaxParticles(maxParticles);
		moduleData->m_Particles.SetMaxParticles(maxParticles);
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
		return GetModuleDataRead()->m_Particles.GetAliveParticles();
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
		glm::mat4 transform = m_Entity.GetComponent<TransformComponent>().WorldTransform;
		Application::Get().GetThreadPool().PushJob<void>([this, timestep, transform, speed]() {
			XYZ_PROFILE_FUNC("ParticleSystemCPU::particleThreadUpdate Job");
			{			
				ScopedLock<ModuleData> moduleData = m_ModuleThreadPass.Get<ModuleData>();
				emit(timestep, moduleData.As(), transform, speed);
				update(timestep, moduleData.As(), transform);
				buildRenderData(moduleData.As());
			}
			m_RenderThreadPass.AttemptSwap();		
		});
	}
	void ParticleSystemCPU::update(Timestep timestep, ModuleData& data, const glm::mat4& transform)
	{
		XYZ_PROFILE_FUNC("ParticleSystemCPU::update");
		data.m_MainModule.UpdateParticles(timestep, data.m_Particles);
		const std::vector<uint32_t>& killed = data.m_MainModule.Killed();

		data.m_PhysicsModule.UpdateParticles(data.m_Particles, transform, killed);
		data.m_LightModule.UpdateParticles(timestep, data.m_Particles);
		data.m_TextureAnimModule.UpdateParticles(timestep, data.m_Particles);
		data.m_RotationOverLife.UpdateParticles(timestep, data.m_Particles);

		for (const uint32_t i : killed)
			data.m_Particles.Kill(i);
	}
	void ParticleSystemCPU::emit(Timestep timestep, ModuleData& data, const glm::mat4& transform, float speed)
	{
		XYZ_PROFILE_FUNC("ParticleSystemCPU::emit");
		data.m_Emitter.Emit(timestep, data.m_Particles);	

		auto [startId, endId] = data.m_Emitter.m_EmittedIDs;
		data.m_PhysicsModule.Generate(data.m_Particles, startId, endId, transform, speed);
	}
	void ParticleSystemCPU::buildRenderData(ModuleData& data)
	{
		XYZ_PROFILE_FUNC("ParticleSystemCPU::buildRenderData");
		ScopedLock<RenderData> val = m_RenderThreadPass.Write();
		ParticleRenderData* buffer = val->m_RenderParticleData.As<ParticleRenderData>();
		const uint32_t endId = data.m_Particles.GetAliveParticles();
		for (uint32_t i = 0; i < endId; ++i)
		{
			const auto& particle = data.m_Particles.m_Particle[i];
			buffer[i] = ParticleRenderData{
				particle.Color,
				particle.Position,
				data.m_Particles.m_Size[i],
				data.m_Particles.m_Rotation[i],
				data.m_Particles.m_TexOffset[i]
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
	ParticleSystemCPU::ModuleData::ModuleData(uint32_t maxParticles)
		:
		m_Particles(maxParticles)
	{
	}
}