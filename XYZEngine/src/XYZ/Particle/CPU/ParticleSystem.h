#pragma once
#include "XYZ/Utils/DataStructures/ThreadPass.h"

#include "XYZ/Core/Timestep.h"
#include "XYZ/Core/Ref/Ref.h"

#include "ParticlePool.h"
#include "ParticleUpdater.h"
#include "ParticleEmitter.h"

#include <glm/glm.hpp>

#include <mutex>

namespace XYZ {

	struct ParticleRenderData
	{
		glm::vec4 Transform[3];
		glm::vec4 Color;
		glm::vec2 TexOffset;
	};

	struct ParticleLightData
	{
		glm::vec3 Color;
		glm::vec3 Position;
		float	  Radius;
		float     Intensity;
	};

	class ParticleSystem : public std::enable_shared_from_this<ParticleSystem>
	{
	public:
		struct RenderData
		{
			RenderData() = default;
			RenderData(uint32_t maxParticles);

			std::vector<ParticleRenderData> ParticleData;
			uint32_t						ParticleCount = 0;

			std::vector<ParticleLightData>	LightData;
		};

	public:
		ParticleSystem(uint32_t maxParticles = 20);	
		ParticleSystem(const ParticleSystem& other);
		ParticleSystem(ParticleSystem&& other) noexcept;
		~ParticleSystem();

		ParticleSystem& operator=(const ParticleSystem& other);
		ParticleSystem& operator=(ParticleSystem&& other) noexcept;

		void Update(const glm::mat4& transform, Timestep ts);	
		void Reset();
		void SetMaxParticles(uint32_t maxParticles);

		uint32_t GetMaxParticles() const;
		uint32_t GetAliveParticles() const;
		
		const RenderData& GetRenderData() const { return m_RenderData; }
		
		ParticleEmitter	Emitter;

		// Texture animation
		glm::ivec2		AnimationTiles;
		uint32_t		AnimationStartFrame;
		float			AnimationCycleLength;

		// Rotation over life
		glm::vec3		RotationEulerAngles;
		float			RotationCycleLength;

		// Color over life
		glm::vec4		StartColor;
		glm::vec4		EndColor;

		// Light over life
		glm::vec3		LightStartColor;
		glm::vec3	    LightEndColor;
		float			LightStartIntensity;
		float			LightEndIntensity;
		float			LightStartRadius;
		float			LightEndRadius;

		float			Speed;
		bool			Play;

		// Enabled
		bool			AnimationEnabled;
		bool			RotationEnabled;
		bool		    LightsEnabled;
		bool			ColorOverLifeEnabled;
		bool			LightOverLifeEnabled;

	private:
		void pushJobs(const glm::mat4& transform, Timestep ts);

		void pushMainJob(Timestep ts);
		void pushRotationJob();
		void pushAnimationJob();
		void pushColorOverLifeJob();
		void pushSizeOverLifeJob();
		void pushBuildLightsDataJob(const glm::mat4& transform);
		void pushBuildRenderDataJobs(const glm::mat4& transform);

		void updateAnimation(ParticlePool::Particle& particle, uint32_t stageCount);
		void updateRotation(ParticlePool::Particle& particle);

		void buildRenderData(const glm::mat4& transform, uint32_t startId, uint32_t endId);

	private:
		ParticlePool		m_Pool;
		RenderData			m_RenderData;
		uint32_t			m_MaxParticles;
		std::shared_mutex	m_JobsMutex;
	
	
		static constexpr uint32_t sc_PerJobCount = 500;
	};

}