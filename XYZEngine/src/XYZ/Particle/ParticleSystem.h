#pragma once
#include "XYZ/Renderer/VertexArray.h"
#include "XYZ/Renderer/Material.h"
#include "XYZ/Core/Ref.h"
#include "XYZ/Core/Timestep.h"

#include "ParticleMaterial.h"
#include "Particle.h"

#include <glm/glm.hpp>

namespace XYZ {

	struct ParticleEmitter
	{
		ParticleEmitter();
		ParticleEmissionShape Shape;
		glm::vec2			  RectangleMin;
		glm::vec2			  RectangleMax;
		glm::vec2		      Center;
		float				  Radius;
	};

	class ParticleSystem : public RefCount
	{
	public:
		ParticleSystem(const Ref<ParticleMaterial>& material);

		void Update(Timestep ts);
		void Reset();

		void GenerateParticleData();
		void SetParticleEmitter(const ParticleEmitter& emitter);
		void SetSpawnRate(float rate) { m_Rate = rate; }
	
		const ParticleEmitter& GetParticleEmitter() const { return m_Emitter; }
		float GetEmittedParticles() const { return m_EmittedParticles; }
		float GetSpawnRate() const { return m_Rate; }
		bool  Playing() const { return m_Playing; }
	
		const Ref<ParticleMaterial>& GetMaterial() const { return m_Material; }
	private:
		Ref<ParticleMaterial> m_Material;
		ParticleEmitter		  m_Emitter;
		float				  m_EmittedParticles;
		float				  m_PlayTime;
		float				  m_Rate;
		bool				  m_Playing;
	};

}