#pragma once
#include "XYZ/Renderer/RendererCommand.h"
#include "XYZ/Core/Ref/Ref.h"
#include "XYZ/Core/Timestep.h"

#include "ParticleMaterial.h"
#include "Particle.h"

#include <glm/glm.hpp>

namespace XYZ {

	struct ParticleEmitterGPU
	{
		ParticleEmitterGPU();
		ParticleEmissionShape Shape;
		glm::vec2			  RectangleMin;
		glm::vec2			  RectangleMax;
		glm::vec2		      Center;
		float				  Radius;
	};

	struct ParticleRendererGPU : public RendererCommand
	{
		virtual void Bind() const override;

		Ref<ParticleMaterial> m_ParticleMaterial;
	};

	class ParticleSystem : public RefCount
	{
	public:
		ParticleSystem(const Ref<ParticleMaterial>& material);

		void Update(Timestep ts);
		void Reset();

		void GenerateParticleData();
		void SetParticleEmitter(const ParticleEmitterGPU& emitter);
		void SetSpawnRate(float rate) { m_Rate = rate; }
	
		const ParticleEmitterGPU& GetParticleEmitter() const { return m_Emitter; }
		float GetEmittedParticles() const { return m_EmittedParticles; }
		float GetSpawnRate() const { return m_Rate; }
		float PlayTime() const { return m_PlayTime; }
		bool  Playing() const { return m_Playing; }
	

		Ref<ParticleRendererGPU> m_Renderer;
	private:
		ParticleEmitterGPU	  m_Emitter;
		float				  m_EmittedParticles;
		float				  m_PlayTime;
		float				  m_Rate;
		bool				  m_Playing;
	};

}