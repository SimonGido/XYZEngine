#pragma once

#include "XYZ/Core/Timestep.h"
#include "ParticleDataBuffer.h"
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
		
		void Emit(Timestep ts, ParticleDataBuffer& data);


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
	private:
		uint32_t burstEmit();
		void	 generateBox(ParticleDataBuffer& data, uint32_t startId, uint32_t endId) const;
		void	 generateCircle(ParticleDataBuffer& data, uint32_t startId, uint32_t endId) const;

	private:
		float m_EmittedParticles;
		float m_PassedTime;
	};
}