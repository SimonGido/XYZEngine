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

			uint32_t m_Count;
			float	 m_Time;

		private:
			bool     m_Called;

			friend struct BurstEmitter;
		};

		BurstEmitter();
		uint32_t Update(Timestep ts);

		std::vector<Burst> m_Bursts;
		float			   m_Interval;
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


		ParticleShapeGenerator		    m_ShapeGenerator;
		ParticleLifeGenerator		    m_LifeGenerator;
		ParticleRandomVelocityGenerator m_RandomVelGenerator;
		BurstEmitter					m_BurstEmitter;
		float							m_EmitRate;

		std::pair<uint32_t, uint32_t>	m_EmittedIDs;
	private:
		float							m_EmittedParticles;
	};
}