#pragma once
#include "XYZ/Core/Ref.h"
#include "ParticleGenerator.h"

#include <mutex>

namespace XYZ {

	class ParticleEmitterCPU : public RefCount
	{
	public:
		ParticleEmitterCPU();
		virtual ~ParticleEmitterCPU() = default;

		virtual void Emit(float timeStep, ParticleDataBuffer* data);
		void		 AddCustomGenerator(ParticleGenerator* generator);
		void		 RemoveCustomGenerator(const ParticleGenerator* generator);

		void         SetEmitRate(float emitRate);
		float        GetEmitRate() const;


		void SetShapeGenerator(const ParticleShapeGenerator& generator);
		void SetLifeGenerator(const ParticleLifeGenerator& generator);
		void SetRandomVelocityGenerator(const ParticleRandomVelocityGenerator& generator);

		ParticleShapeGenerator			GetShapeGenerator() const;
		ParticleLifeGenerator			GetLifeGenerator() const;
		ParticleRandomVelocityGenerator GetRandomVelocityGenerator() const;

	private:
		void removeGenerator(const ParticleGenerator* generator);


	protected:
		mutable std::mutex m_Mutex;
		float			   m_EmittedParticles;
	

	private:
		std::vector<ParticleGenerator*> m_Generators;


		ParticleShapeGenerator			m_ShapeGenerator;
		ParticleLifeGenerator			m_LifeGenerator;
		ParticleRandomVelocityGenerator m_RandomVelocityGenerator;

		float m_EmitRate;
	};

}