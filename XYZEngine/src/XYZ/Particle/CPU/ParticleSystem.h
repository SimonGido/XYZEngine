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
		glm::vec4 Color;
		glm::vec3 Position;
		glm::vec3 Size;
		glm::quat Axis;
		glm::vec2 TexOffset;
	};

	struct ParticleLightData
	{
		glm::vec3 Color;
		glm::vec3 Position;
		float	  Radius;
		float     Intensity;
	};

	class ParticleSystem
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

		void Update(Timestep ts);	
		void Reset();
		void SetMaxParticles(uint32_t maxParticles);

		uint32_t GetMaxParticles() const;
		uint32_t GetAliveParticles() const;
		

		ScopedLock<RenderData>		GetRenderData();
		ScopedLockRead<RenderData>	GetRenderDataRead() const;

		ParticleEmitter	Emitter;
		glm::ivec2		AnimationTiles;
		uint32_t		AnimationStartFrame;
		float			AnimationCycleLength;

		glm::vec3		RotationEulerAngles;
		float			RotationCycleLength;

		float			Speed;
		bool			Play;

	private:
		void particleThreadUpdate(float timestep);
		void update(Timestep timestep);
		void buildRenderData();

	private:
		ParticlePool					m_Pool;
		ThreadPass<RenderData>			m_RenderThreadPass;
		uint32_t						m_MaxParticles;
	};

}