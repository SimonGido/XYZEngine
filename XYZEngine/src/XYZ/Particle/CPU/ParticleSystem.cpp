#include "stdafx.h"
#include "ParticleSystem.h"
#include "XYZ/Renderer/SceneRenderer.h"

#include "XYZ/Scene/Components.h"
#include "XYZ/Core/Application.h"

#include "XYZ/Debug/Profiler.h"

#include "XYZ/Utils/Math/Math.h"

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/compatibility.hpp>

namespace XYZ {
	static float CalcRatio(float length, float value)
	{
		return (length - value) / length;
	}

	void Mat4ToTransformData(glm::vec4* transformRows, const glm::mat4& transform)
	{
		RenderQueue::TransformData data;
		transformRows[0] = { transform[0][0], transform[1][0], transform[2][0], transform[3][0] };
		transformRows[1] = { transform[0][1], transform[1][1], transform[2][1], transform[3][1] };
		transformRows[2] = { transform[0][2], transform[1][2], transform[2][2], transform[3][2] };
	}

	ParticleSystem::ParticleSystem(uint32_t maxParticles)
		:
		AnimationTiles(1, 1),
		AnimationStartFrame(0),
		AnimationCycleLength(0.0f),
		EndRotation(0.0f),
		EndSize(1.0f),
		EndColor(1.0f),
		LightEndColor(1.0f),
		LightEndIntensity(1.0f),
		LightEndRadius(1.0f),
		Play(true),
		Speed(1.0f),
		m_Pool(maxParticles),
		m_RenderData(maxParticles),
		m_MaxParticles(maxParticles)
	{
		for (auto& enabled : ModuleEnabled)
			enabled = false;
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
		EndRotation(other.EndRotation),
		EndSize(other.EndSize),
		EndColor(other.EndColor),
		LightEndColor(other.LightEndColor),
		LightEndIntensity(other.LightEndIntensity),
		LightEndRadius(other.LightEndRadius),
		Play(other.Play),
		Speed(other.Speed),
		m_Pool(other.m_Pool),
		m_RenderData(other.m_RenderData),
		m_MaxParticles(other.m_MaxParticles)
	{
		for (uint32_t i = 0; i < NumModules; ++i)
			ModuleEnabled[i] = other.ModuleEnabled[i];
	}

	ParticleSystem::ParticleSystem(ParticleSystem&& other) noexcept
		:
		Emitter(other.Emitter),
		AnimationTiles(other.AnimationTiles),
		AnimationStartFrame(other.AnimationStartFrame),
		AnimationCycleLength(other.AnimationCycleLength),
		EndRotation(other.EndRotation),
		EndSize(other.EndSize),
		EndColor(other.EndColor),
		LightEndColor(other.LightEndColor),
		LightEndIntensity(other.LightEndIntensity),
		LightEndRadius(other.LightEndRadius),
		Play(other.Play),
		Speed(other.Speed),
		m_RenderData(std::move(other.m_RenderData)),
		m_Pool(std::move(other.m_Pool)),
		m_MaxParticles(other.m_MaxParticles)
	{
		for (uint32_t i = 0; i < NumModules; ++i)
			ModuleEnabled[i] = other.ModuleEnabled[i];
	}

	ParticleSystem& ParticleSystem::operator=(const ParticleSystem& other)
	{	
		Emitter = other.Emitter;
		AnimationTiles = other.AnimationTiles;
		AnimationStartFrame = other.AnimationStartFrame;
		EndRotation = other.EndRotation;
		EndSize = other.EndSize;
		EndColor = other.EndColor;
		LightEndColor = other.LightEndColor;
		LightEndIntensity = other.LightEndIntensity;
		LightEndRadius = other.LightEndRadius;

		Play = other.Play;
		Speed = other.Speed;
		
		for (uint32_t i = 0; i < NumModules; ++i)
			ModuleEnabled[i] = other.ModuleEnabled[i];
		
		std::unique_lock lock(m_JobsMutex);
		m_RenderData = other.m_RenderData;
		m_Pool = other.m_Pool;
		m_MaxParticles = other.m_MaxParticles;

		return *this;
	}

	ParticleSystem& ParticleSystem::operator=(ParticleSystem&& other) noexcept
	{
		Emitter = other.Emitter;
		AnimationTiles = other.AnimationTiles;
		AnimationStartFrame = other.AnimationStartFrame;
		EndRotation = other.EndRotation;
		EndSize = other.EndSize;
		EndColor = other.EndColor;
		LightEndColor = other.LightEndColor;
		LightEndIntensity = other.LightEndIntensity;
		LightEndRadius = other.LightEndRadius;
		Play = other.Play;
		Speed = other.Speed;
		for (uint32_t i = 0; i < NumModules; ++i)
			ModuleEnabled[i] = other.ModuleEnabled[i];

		std::unique_lock lock(m_JobsMutex);
		m_RenderData = std::move(other.m_RenderData);
		m_Pool = std::move(other.m_Pool);
		m_MaxParticles = other.m_MaxParticles;
		return *this;
	}

	void ParticleSystem::Update(const glm::mat4& transform, Timestep ts)
	{	
		XYZ_PROFILE_FUNC("ParticleSystem::Update");
		if (Play)
		{		
			pushJobs(transform, ts * Speed);
		}
	}

	void ParticleSystem::Reset()
	{
		std::unique_lock lock(m_JobsMutex);
		for (uint32_t i = 0; i < m_Pool.GetAliveParticles(); ++i)
			m_Pool.Kill(i);
	}

	void ParticleSystem::SetMaxParticles(uint32_t maxParticles)
	{
		std::unique_lock lock(m_JobsMutex);

		m_RenderData.ParticleData.resize(maxParticles);
		m_Pool.SetMaxParticles(maxParticles);
		m_MaxParticles = maxParticles;
	}

	uint32_t ParticleSystem::GetMaxParticles() const
	{
		return m_MaxParticles;
	}

	uint32_t ParticleSystem::GetAliveParticles() const
	{
		return m_Pool.GetAliveParticles();
	}
	void ParticleSystem::pushJobs(const glm::mat4& transform, Timestep ts)
	{
		XYZ_PROFILE_FUNC("ParticleSystem::pushJobsManyThreads");
		if (m_MaxParticles == 0)
			return;

		std::unique_lock lock(m_JobsMutex);
		pushMainJob(ts);
		
		if (ModuleEnabled[RotationOverLife])
			pushRotationJob();
		if (ModuleEnabled[SizeOverLife])
			pushSizeOverLifeJob();
		if (ModuleEnabled[ColorOverLife])
			pushColorOverLifeJob();
		if (ModuleEnabled[TextureAnimation])
			pushAnimationJob();	
		if (ModuleEnabled[LightOverLife])
			pushLightOverLifeJob();


		pushBuildLightsDataJob(transform);
		pushBuildRenderDataJobs(transform);
	}

	void ParticleSystem::pushMainJob(Timestep ts)
	{
		std::shared_ptr<ParticleSystem> instance = shared_from_this();
		Application::Get().GetThreadPool().PushJob<void>([instance, ts]() {

			XYZ_PROFILE_FUNC("ParticleSystem::pushMainJob");
			std::unique_lock lock(instance->m_JobsMutex);

			instance->Emitter.Kill(instance->m_Pool);
			instance->Emitter.Emit(ts, instance->m_Pool);

			const uint32_t aliveParticles = instance->m_Pool.GetAliveParticles();
			for (uint32_t i = 0; i < aliveParticles; ++i)
			{
				instance->m_Pool.Particles[i].Position += instance->m_Pool.Particles[i].Velocity * ts.GetSeconds();
				instance->m_Pool.Particles[i].LifeRemaining -= ts.GetSeconds();
			}
		});
	}

	void ParticleSystem::pushRotationJob()
	{
		std::shared_ptr<ParticleSystem> instance = shared_from_this();
		Application::Get().GetThreadPool().PushJob<void>([instance]() {

			XYZ_PROFILE_FUNC("ParticleSystem::pushRotationJob");
			std::shared_lock lock(instance->m_JobsMutex);

			auto particles = instance->m_Pool.Particles;
			const uint32_t aliveParticles = instance->m_Pool.GetAliveParticles();

			for (uint32_t i = 0; i < aliveParticles; ++i)
			{
				instance->updateRotation(particles[i]);
			}
		});
	}

	void ParticleSystem::pushAnimationJob()
	{
		std::shared_ptr<ParticleSystem> instance = shared_from_this();
		Application::Get().GetThreadPool().PushJob<void>([instance]() {

			XYZ_PROFILE_FUNC("ParticleSystem::pushAnimationJob");
			std::shared_lock lock(instance->m_JobsMutex);

			auto particles = instance->m_Pool.Particles;
			const uint32_t aliveParticles = instance->m_Pool.GetAliveParticles();

			const uint32_t stageCount = instance->AnimationTiles.x * instance->AnimationTiles.y;
			for (uint32_t i = 0; i < aliveParticles; ++i)
			{
				instance->updateAnimation(particles[i], stageCount);
			}
		});
	}

	void ParticleSystem::pushColorOverLifeJob()
	{
		std::shared_ptr<ParticleSystem> instance = shared_from_this();
		Application::Get().GetThreadPool().PushJob<void>([instance]() {

			XYZ_PROFILE_FUNC("ParticleSystem::pushColorOverLifeJob");
			std::shared_lock lock(instance->m_JobsMutex);

			auto particles = instance->m_Pool.Particles;
			const uint32_t aliveParticles = instance->m_Pool.GetAliveParticles();

			for (uint32_t i = 0; i < aliveParticles; ++i)
			{
				instance->updateColorOverLife(particles[i]);
			}
		});
	}

	void ParticleSystem::pushSizeOverLifeJob()
	{
		std::shared_ptr<ParticleSystem> instance = shared_from_this();
		Application::Get().GetThreadPool().PushJob<void>([instance]() {

			XYZ_PROFILE_FUNC("ParticleSystem::pushSizeOverLifeJob");
			std::shared_lock lock(instance->m_JobsMutex);

			auto particles = instance->m_Pool.Particles;
			const uint32_t aliveParticles = instance->m_Pool.GetAliveParticles();

			for (uint32_t i = 0; i < aliveParticles; ++i)
			{
				instance->updateSizeOverLife(particles[i]);
			}
		});
	}

	void ParticleSystem::pushLightOverLifeJob()
	{
		std::shared_ptr<ParticleSystem> instance = shared_from_this();
		Application::Get().GetThreadPool().PushJob<void>([instance]() {

			XYZ_PROFILE_FUNC("ParticleSystem::pushLightOverLifeJob");
			std::shared_lock lock(instance->m_JobsMutex);

			auto particles = instance->m_Pool.Particles;
			const uint32_t aliveParticles = instance->m_Pool.GetAliveParticles();

			const uint32_t aliveLights = std::min(aliveParticles, instance->Emitter.MaxLights);

			for (uint32_t i = 0; i < aliveLights; ++i)
			{
				instance->updateLightOverLife(particles[i]);
			}
		});
	}

	void ParticleSystem::pushBuildLightsDataJob(const glm::mat4& transform)
	{
		std::shared_ptr<ParticleSystem> instance = shared_from_this();
		Application::Get().GetThreadPool().PushJob<void>([instance,tr = transform]() {

			XYZ_PROFILE_FUNC("ParticleSystem::pushBuildLightsDataJob");
			std::shared_lock lock(instance->m_JobsMutex);
			

			const uint32_t aliveParticles = instance->m_Pool.GetAliveParticles();

			const uint32_t maxLights = std::min(aliveParticles, instance->Emitter.MaxLights);
			instance->m_RenderData.LightData.resize(maxLights);
			for (uint32_t i = 0; i < maxLights; ++i)
			{
				const auto& particle = instance->m_Pool.Particles[i];

				const glm::mat4 particleTransform =
					glm::translate(particle.Position)
				  * glm::toMat4(particle.Rotation)
				  * glm::scale(particle.Size);

				const glm::mat4 worldParticleTransform = tr * particleTransform;

				auto& light = instance->m_RenderData.LightData[i];
				light.Color = particle.LightColor;
				light.Position = Math::TransformToTranslation(worldParticleTransform);
				light.Radius = particle.LightRadius;
				light.Intensity = particle.LightIntensity;
			}
		});
	}

	void ParticleSystem::pushBuildRenderDataJobs(const glm::mat4& transform)
	{
		std::shared_ptr<ParticleSystem> instance = shared_from_this();
		
		const uint32_t aliveParticles = instance->m_Pool.GetAliveParticles();
		const uint32_t numJobs = aliveParticles / sc_PerJobCount;
		for (uint32_t jobIndex = 0; jobIndex < numJobs + 1; ++jobIndex)
		{
			Application::Get().GetThreadPool().PushJob<void>([instance, jobIndex, tr = transform]() {

				XYZ_PROFILE_FUNC("ParticleSystem::pushBuildRenderDataJob");
				std::shared_lock lock(instance->m_JobsMutex);
	
				const uint32_t aliveParticles = instance->m_Pool.GetAliveParticles();

				const uint32_t startId = jobIndex * instance->sc_PerJobCount;
				const uint32_t endId = std::min(startId + instance->sc_PerJobCount, aliveParticles);

				instance->buildRenderData(tr, startId, endId);
				instance->m_RenderData.ParticleCount = aliveParticles;
			});
		}
	}



	void ParticleSystem::updateAnimation(ParticlePool::Particle& particle, uint32_t stageCount) const
	{
		const float ratio = CalcRatio(AnimationCycleLength, particle.LifeRemaining);
		const float stageProgress = ratio * stageCount;

		const uint32_t index = (uint32_t)floor(stageProgress);
		const float column = index % AnimationTiles.x;
		const float row = index / AnimationTiles.y;

		particle.TexOffset = glm::vec2(column / (float)AnimationTiles.x, row / (float)AnimationTiles.y);
	}

	void ParticleSystem::updateRotation(ParticlePool::Particle& particle) const
	{
		const glm::vec3 radians = glm::radians(EndRotation);
		const float ratio = CalcRatio(Emitter.LifeTime, Emitter.LifeTime - particle.LifeRemaining);
		particle.Rotation = glm::quat(radians * ratio);
	}

	void ParticleSystem::updateColorOverLife(ParticlePool::Particle& particle) const
	{
		const float ratio = CalcRatio(Emitter.LifeTime, Emitter.LifeTime - particle.LifeRemaining);
		particle.Color = glm::lerp(Emitter.Color, EndColor, ratio);
	}

	void ParticleSystem::updateSizeOverLife(ParticlePool::Particle& particle) const
	{
		const float ratio = CalcRatio(Emitter.LifeTime, Emitter.LifeTime - particle.LifeRemaining);
		particle.Size = glm::lerp(Emitter.Size, EndSize, ratio);
	}

	void ParticleSystem::updateLightOverLife(ParticlePool::Particle& particle) const
	{
		const float ratio = CalcRatio(Emitter.LifeTime, Emitter.LifeTime - particle.LifeRemaining);
		particle.LightColor = glm::lerp(Emitter.LightColor, LightEndColor, ratio);
		particle.LightIntensity = glm::lerp(Emitter.LightIntensity, LightEndIntensity, ratio);
		particle.LightRadius = glm::lerp(Emitter.LightRadius, LightEndRadius, ratio);
	}

	void ParticleSystem::buildRenderData(const glm::mat4& transform, uint32_t startId, uint32_t endId)
	{
		XYZ_PROFILE_FUNC("ParticleSystem::buildRenderData");
		for (uint32_t i = startId; i < endId; ++i)
		{
			const auto& particle = m_Pool.Particles[i];

			const glm::mat4 particleTransform =
				glm::translate(particle.Position)
				* glm::toMat4(particle.Rotation)
				* glm::scale(particle.Size);
			
			
			const glm::mat4 worldParticleTransform = transform * particleTransform;


			m_RenderData.ParticleData[i].Color = particle.Color;
			Mat4ToTransformData(m_RenderData.ParticleData[i].Transform, worldParticleTransform);
			m_RenderData.ParticleData[i].TexOffset = particle.TexOffset;
		}
	}

	ParticleSystem::RenderData::RenderData(uint32_t maxParticles)
	{
		ParticleData.resize(maxParticles);
	}
}