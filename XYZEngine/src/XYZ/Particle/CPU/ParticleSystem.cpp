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
		Speed(1.0f)
	{
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
		Speed(other.Speed)
	{
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
		Speed(other.Speed)
	{

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
	
		return *this;
	}


	void ParticleSystem::Update(Timestep ts, ParticlePool& pool)
	{		
		m_StartParticle = pool.GetAliveParticles();
		Emitter.Update(ts, pool);
		m_EndParticle = pool.GetAliveParticles();	
	}

	void ParticleSystem::UpdateRotation(ParticlePool& pool)
	{
		XYZ_PROFILE_FUNC("ParticleSystem::UpdateRotation");

		auto particles = pool.Particles;
		const uint32_t aliveParticles = pool.GetAliveParticles();

		for (uint32_t i = m_StartParticle; i < m_EndParticle; ++i)
		{
			updateRotation(particles[i]);
		}
	}

	void ParticleSystem::UpdateAnimation(ParticlePool& pool)
	{
		XYZ_PROFILE_FUNC("ParticleSystem::UpdateAnimation");

		auto particles = pool.Particles;
		const uint32_t aliveParticles = pool.GetAliveParticles();

		const uint32_t stageCount = AnimationTiles.x * AnimationTiles.y;
		for (uint32_t i = m_StartParticle; i < m_EndParticle; ++i)
		{
			updateAnimation(particles[i], stageCount);
		}
	}

	void ParticleSystem::UpdateColorOverLife(ParticlePool& pool)
	{
	}

	void ParticleSystem::UpdateSizeOverLife(ParticlePool& pool)
	{
	}

	void ParticleSystem::UpdateLightOverLife(ParticlePool& pool)
	{
	}


	void ParticleSystem::pushJobs(const glm::mat4& transform, Timestep ts)
	{

	}

	void ParticleSystem::pushMainJob(Timestep ts)
	{
		
	}

	void ParticleSystem::pushRotationJob()
	{
		
	}

	void ParticleSystem::pushAnimationJob()
	{
		
	}

	void ParticleSystem::pushColorOverLifeJob()
	{
	
	}

	void ParticleSystem::pushSizeOverLifeJob()
	{
		//m_JobsCount++;
		//Ref<ParticleSystem> instance = this;
		//Application::Get().GetThreadPool().PushJob([instance]() mutable {
		//
		//	XYZ_PROFILE_FUNC("ParticleSystem::pushSizeOverLifeJob");
		//	std::shared_lock lock(instance->m_JobsMutex);
		//
		//	auto particles = instance->m_Pool.Particles;
		//	const uint32_t aliveParticles = instance->m_Pool.GetAliveParticles();
		//
		//	for (uint32_t i = 0; i < aliveParticles; ++i)
		//	{
		//		instance->updateSizeOverLife(particles[i]);
		//	}
		//	instance->m_JobsCount--;
		//});
	}

	void ParticleSystem::pushLightOverLifeJob()
	{
		//m_JobsCount++;
		//Ref<ParticleSystem> instance = this;
		//Application::Get().GetThreadPool().PushJob([instance]() mutable {
		//
		//	XYZ_PROFILE_FUNC("ParticleSystem::pushLightOverLifeJob");
		//	std::shared_lock lock(instance->m_JobsMutex);
		//
		//	auto particles = instance->m_Pool.Particles;
		//	const uint32_t aliveParticles = instance->m_Pool.GetAliveParticles();
		//
		//	const uint32_t aliveLights = std::min(aliveParticles, instance->Emitter.MaxLights);
		//
		//	for (uint32_t i = 0; i < aliveLights; ++i)
		//	{
		//		instance->updateLightOverLife(particles[i]);
		//	}
		//	instance->m_JobsCount--;
		//});
	}

	void ParticleSystem::pushBuildLightsDataJob(const glm::mat4& transform)
	{
		//m_JobsCount++;
		//Ref<ParticleSystem> instance = this;
		//Application::Get().GetThreadPool().PushJob([instance,tr = transform]() mutable {
		//
		//	XYZ_PROFILE_FUNC("ParticleSystem::pushBuildLightsDataJob");
		//	std::shared_lock lock(instance->m_JobsMutex);
		//	
		//
		//	const uint32_t aliveParticles = instance->m_Pool.GetAliveParticles();
		//
		//	const uint32_t maxLights = std::min(aliveParticles, instance->Emitter.MaxLights);
		//	instance->m_RenderData.LightData.resize(maxLights);
		//	for (uint32_t i = 0; i < maxLights; ++i)
		//	{
		//		const auto& particle = instance->m_Pool.Particles[i];
		//
		//		const glm::mat4 particleTransform =
		//			glm::translate(particle.Position)
		//		  * glm::toMat4(particle.Rotation)
		//		  * glm::scale(particle.Size);
		//
		//		const glm::mat4 worldParticleTransform = tr * particleTransform;
		//
		//		auto& light = instance->m_RenderData.LightData[i];
		//		light.Color = particle.LightColor;
		//		light.Position = Math::TransformToTranslation(worldParticleTransform);
		//		light.Radius = particle.LightRadius;
		//		light.Intensity = particle.LightIntensity;
		//	}
		//	instance->m_JobsCount--;
		//});
	}

	void ParticleSystem::pushBuildRenderDataJobs(const glm::mat4& transform)
	{	
		
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
		const float ratio = CalcRatio(Emitter.LifeTime, particle.LifeRemaining);
		particle.Rotation = glm::quat(radians * ratio);
	}

	void ParticleSystem::updateColorOverLife(ParticlePool::Particle& particle) const
	{
		const float ratio = CalcRatio(Emitter.LifeTime, particle.LifeRemaining);
		particle.Color = glm::lerp(Emitter.Color, EndColor, ratio);
	}

	void ParticleSystem::updateSizeOverLife(ParticlePool::Particle& particle) const
	{
		const float ratio = CalcRatio(Emitter.LifeTime, particle.LifeRemaining);
		particle.Size = glm::lerp(Emitter.Size, EndSize, ratio);
	}

	void ParticleSystem::updateLightOverLife(ParticlePool::Particle& particle) const
	{
		const float ratio = CalcRatio(Emitter.LifeTime, particle.LifeRemaining);
		particle.LightColor = glm::lerp(Emitter.LightColor, LightEndColor, ratio);
		particle.LightIntensity = glm::lerp(Emitter.LightIntensity, LightEndIntensity, ratio);
		particle.LightRadius = glm::lerp(Emitter.LightRadius, LightEndRadius, ratio);
	}
}