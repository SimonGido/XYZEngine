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


	class ParticleSystem : public RefCount
	{
	public:
		ParticleSystem(uint32_t maxParticles = 20);	
		ParticleSystem(const ParticleSystem& other);
		ParticleSystem(ParticleSystem&& other) noexcept;
		~ParticleSystem();

		ParticleSystem& operator=(const ParticleSystem& other);
		ParticleSystem& operator=(ParticleSystem&& other) noexcept;

		// New API //
		void Update(Timestep ts, ParticlePool& pool);
		void UpdateRotation(ParticlePool& pool);
		void UpdateAnimation(ParticlePool& pool);
		void UpdateColorOverLife(ParticlePool& pool);
		void UpdateSizeOverLife(ParticlePool& pool);
		void UpdateLightOverLife(ParticlePool& pool);
		/////////////
		ParticleEmitter	Emitter;
		uint32_t		MaxParticles;

		// Texture animation
		glm::ivec2		AnimationTiles;
		uint32_t		AnimationStartFrame;
		float			AnimationCycleLength;

		// Rotation over life
		glm::vec3		EndRotation; // Euler angles
	
		// Size over life
		glm::vec3		EndSize;

		// Color over life
		glm::vec4		EndColor;

		// Light over life
		glm::vec3	    LightEndColor;
		float			LightEndIntensity;
		float			LightEndRadius;

		float			Speed;
		bool			Play;

	private:
		void pushJobs(const glm::mat4& transform, Timestep ts);

		void pushMainJob(Timestep ts);
		void pushRotationJob();
		void pushAnimationJob();
		void pushColorOverLifeJob();
		void pushSizeOverLifeJob();
		void pushLightOverLifeJob();
		void pushBuildLightsDataJob(const glm::mat4& transform);
		void pushBuildRenderDataJobs(const glm::mat4& transform);

		void updateAnimation(ParticlePool::Particle& particle, uint32_t stageCount) const;
		void updateRotation(ParticlePool::Particle& particle) const;
		void updateColorOverLife(ParticlePool::Particle& particle) const;
		void updateSizeOverLife(ParticlePool::Particle& particle) const;
		void updateLightOverLife(ParticlePool::Particle& particle) const;

	public:
		
		uint32_t m_StartParticle = 0;
		uint32_t m_EndParticle	= 0;
	};



}