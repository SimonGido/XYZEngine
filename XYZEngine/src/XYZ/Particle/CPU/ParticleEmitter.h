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
		void         AddGenerator(const Ref<ParticleGenerator>& generator);
		void         SetEmitRate(float emitRate);
		float        GetEmitRate() const;
	protected:
		mutable std::mutex m_Mutex;
		float			   m_EmittedParticles;

	private:
		std::vector<Ref<ParticleGenerator>> m_Generators;
		float m_EmitRate;
	};

}