#pragma once

#include "XYZ/Core/Timestep.h"
#include "ParticleDataBuffer.h"
#include "ParticleGenerator.h"

namespace XYZ {

	

	struct BurstEmitter
	{
		struct Burst
		{
			Burst();
			Burst(uint32_t count, float time);

			uint32_t Count;
			float	 Time;

		private:
			bool     m_Called;

			friend struct BurstEmitter;
		};

		BurstEmitter();
		uint32_t Update(Timestep ts);

		std::vector<Burst> Bursts;
		float			   Interval;
	private:
		void reset();

	private:
		float			   m_PassedTime;
	};


	class ParticleEmitterCPU
	{
	public:
		ParticleEmitterCPU();

		void Emit(Timestep ts, ParticleDataBuffer& particles);


		ParticleShapeGenerator		    ShapeGenerator;
		ParticleLifeGenerator		    LifeGenerator;
		ParticleRandomVelocityGenerator RandomVelGenerator;
		BurstEmitter					BurstEmitter;
		float							EmitRate;

	private:
		float							m_EmittedParticles;
	};
}