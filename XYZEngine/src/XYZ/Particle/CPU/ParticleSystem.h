#pragma once
#include "XYZ/Utils/DataStructures/ThreadPass.h"

#include "XYZ/Core/Timestep.h"
#include "XYZ/Core/Ref/Ref.h"

#include "ParticleDataBuffer.h"
#include "ParticleUpdater.h"
#include "ParticleGenerator.h"
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

	class ParticleSystem
	{
	public:
		struct ModuleData
		{
			ModuleData(uint32_t maxParticles);

			// Emitter
			ParticleEmitter					Emitter;
			
			// Generators
			ParticleGenerator				Main;
			ParticleShapeGenerator		    Shape;
			ParticleLifeGenerator		    Life;
			ParticleRandomVelocityGenerator RandomVelocity;

			// Updaters
			LightUpdater					Light;
			TextureAnimationUpdater			TextureAnim;
			RotationOverLife				RotationOverLife;

			
			ParticleDataBuffer				Particles;
		};
		struct RenderData
		{
			RenderData() = default;
			RenderData(uint32_t maxParticles);

			std::vector<ParticleRenderData> Data;
			uint32_t						InstanceCount = 0;
		};

	public:
		ParticleSystem();
		ParticleSystem(uint32_t maxParticles);	
		ParticleSystem(const ParticleSystem& other);
		ParticleSystem(ParticleSystem&& other) noexcept;
		~ParticleSystem();

		ParticleSystem& operator=(const ParticleSystem& other);
		ParticleSystem& operator=(ParticleSystem&& other) noexcept;

		void Update(Timestep ts);	
		void Play();
		void Stop();
		void Reset();
		void SetMaxParticles(uint32_t maxParticles);
		void SetSpeed(float speed);

		uint32_t GetMaxParticles() const;
		uint32_t GetAliveParticles() const;
		float    GetSpeed() const;

		ScopedLock<ModuleData>		GetModuleData();
		ScopedLockRead<ModuleData>	GetModuleDataRead() const;

		ScopedLock<RenderData>		GetRenderData();
		ScopedLockRead<RenderData>	GetRenderDataRead() const;

	private:
		void particleThreadUpdate(float timestep);
		void update(Timestep timestep, ModuleData& data);
		void buildRenderData(ModuleData& data);

	private:
		SingleThreadPass<ModuleData>		 m_ModuleThreadPass;
		ThreadPass<RenderData>				 m_RenderThreadPass;
		uint32_t							 m_MaxParticles;
		bool								 m_Play;
		float								 m_Speed;
	};

}