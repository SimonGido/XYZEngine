#pragma once
#include "XYZ/Core/Timestep.h"
#include "ParticleEmitterModuleGPU.h"

#include <glm/glm.hpp>

namespace XYZ {


	class ParticleEmitterGPU : public RefCount
	{
	public:
		virtual void	 Emit(uint32_t count, std::byte* buffer) = 0;
		virtual uint32_t EmitCount(Timestep ts) = 0;

		virtual void	 SetEmissionRate(float rate) = 0;

		virtual uint32_t GetStride() const = 0;
		virtual float	 GetEmissionRate() const = 0;
	};



	class DefaultParticleEmitterGPU : public ParticleEmitterGPU
	{
	public:
		DefaultParticleEmitterGPU();

		virtual void	 Emit(uint32_t count, std::byte* buffer) override;
		virtual uint32_t EmitCount(Timestep ts) override;
		virtual void	 SetEmissionRate(float rate) override;


		virtual uint32_t GetStride() const override;
		virtual float	 GetEmissionRate() const override;


		BoxParticleEmitterModuleGPU BoxEmitterModule;

	private:
		float m_EmissionRate;
		float m_Emitted;
	};
}