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
		RotationEulerAngles(0.0f),
		RotationCycleLength(0.0f),
		Play(true),
		Speed(1.0f),
		m_Pool(maxParticles),
		m_RenderData(maxParticles),
		m_MaxParticles(maxParticles)
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
		RotationEulerAngles(other.RotationEulerAngles),
		RotationCycleLength(other.RotationCycleLength),
		Play(other.Play),
		Speed(other.Speed),
		m_Pool(other.m_Pool),
		m_RenderData(other.m_RenderData),
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
		m_RenderData(std::move(other.m_RenderData)),
		m_Pool(std::move(other.m_Pool)),
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
		RotationEulerAngles = other.RotationEulerAngles;
		RotationCycleLength = other.RotationCycleLength;
		Play = other.Play;
		Speed = other.Speed;

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
		XYZ_PROFILE_FUNC("ParticleSystem::pushJobs");
		if (m_MaxParticles == 0)
			return;


		std::unique_lock lock(m_JobsMutex); // We must sync with main thread or it will keep filling thread pool
		std::shared_ptr<ParticleSystem> instance = shared_from_this();
		// Kill old and emit new job
		Application::Get().GetThreadPool().PushJob<void>([instance, ts, tr = transform]() {

			XYZ_PROFILE_FUNC("ParticleSystem::pushJobs emit");
			std::unique_lock lock(instance->m_JobsMutex);

			instance->Emitter.Kill(instance->m_Pool);
			instance->Emitter.Emit(ts, instance->m_Pool);

			const uint32_t aliveParticles = instance->m_Pool.GetAliveParticles();
			instance->update(ts, 0, aliveParticles);
			instance->buildRenderData(tr, 0, aliveParticles);
			instance->m_RenderData.ParticleCount = aliveParticles;
		});
	}


	void ParticleSystem::update(Timestep ts, uint32_t startId, uint32_t endId)
	{
		XYZ_PROFILE_FUNC("ParticleSystem::update");
		auto particles = m_Pool.Particles;


		const uint32_t stageCount = AnimationTiles.x * AnimationTiles.y;
		const float columnSize = 1.0f / AnimationTiles.x;
		const float rowSize = 1.0f / AnimationTiles.y;


		for (uint32_t i = startId; i < endId; ++i)
		{
			particles[i].Position += particles[i].Velocity * ts.GetSeconds();
			particles[i].LifeRemaining -= ts.GetSeconds();

			const float ratio = CalcRatio(AnimationCycleLength, particles[i].LifeRemaining);
			const float stageProgress = ratio * stageCount;

			const uint32_t index = (uint32_t)floor(stageProgress);
			const float column = index % AnimationTiles.x;
			const float row = index / AnimationTiles.y;

			particles[i].TexOffset = glm::vec2(column / (float)AnimationTiles.x, row / (float)AnimationTiles.y);
		}


		//data.TextureAnimationUpdater.UpdateParticles(timestep, data.Pool);
		//data.RotationOverLifeUpdater.UpdateParticles(timestep, data.Pool);

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
		//val->LightData.clear();
		//val->LightData.reserve(Emitter.MaxLights);
		//for (uint32_t i = 0; i < endId && i < Emitter.MaxLights; ++i)
		//{
		//	const auto& particle = m_Pool.Particles[i];
		//	auto& light = val->LightData.emplace_back();
		//	light.Color = particle.LightColor;
		//	light.Position = particle.Position;
		//	light.Radius = particle.LightRadius;
		//	light.Intensity = particle.LightIntensity;
		//}
		
	}

	ParticleSystem::RenderData::RenderData(uint32_t maxParticles)
	{
		ParticleData.resize(maxParticles);
	}
}