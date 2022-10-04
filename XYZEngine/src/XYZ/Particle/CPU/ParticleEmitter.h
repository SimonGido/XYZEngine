#pragma once

#include "XYZ/Core/Timestep.h"
#include "ParticlePool.h"
#include "ParticleUpdater.h"

namespace XYZ {


	enum class EmitShape
	{
		None,
		Box,
		Circle
	};

	struct EmitterBurst
	{
		EmitterBurst() = default;
		EmitterBurst(uint32_t count, float time, float probability);

		uint32_t Count = 0;
		float	 Time = 0.0f;
		float    Probability = 1.0f;
		bool	 Ready = true;
	};


	class ParticleEmitter
	{
	public:
		ParticleEmitter();
		
		void Update(Timestep ts, ParticlePool& data);
		void Emit(uint32_t count, ParticlePool& data, const glm::vec3& position);
		void Kill(ParticlePool& data);

		EmitShape				  Shape;
		glm::vec3				  BoxMin;
		glm::vec3				  BoxMax;
		float					  Radius;

		float					  EmitRate;
		float					  LifeTime;

		glm::vec3				  MinVelocity;
		glm::vec3				  MaxVelocity;
		glm::vec3				  Size;
		glm::vec4				  Color;

		float					  BurstInterval;
		std::vector<EmitterBurst> Bursts;

		uint32_t				  MaxLights;
		glm::vec3				  LightColor;
		float					  LightRadius;
		float					  LightIntensity;
	private:
		uint32_t burstEmit();

		void	 generate(ParticlePool& data, const glm::vec3& position, uint32_t id) const;
		void	 generateBox(ParticlePool& data, const glm::vec3& position, uint32_t id) const;
		void	 generateCircle(ParticlePool& data, const glm::vec3& position, uint32_t id) const;

	private:
		float m_EmittedParticles;
		float m_PassedTime;


		friend class ParticleSystem;
	};
}