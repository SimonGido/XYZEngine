#pragma once
#include "XYZ/Renderer/VertexArray.h"
#include "XYZ/Renderer/Material.h"
#include "XYZ/Core/Ref.h"
#include "XYZ/Core/Timestep.h"

#include "Particle.h"

#include <glm/glm.hpp>

namespace XYZ {
	
	struct ParticleConfiguration
	{
		ParticleConfiguration();

		glm::vec4 ColorRatio;
		glm::vec2 SizeRatio;
		glm::vec2 VelocityRatio;
		uint32_t  MaxParticles;
		float     Rate;
		float	  Gravity;
		float     Speed;
		float     Time;
		bool	  Repeat;
	};

	class ParticleSystem
	{
	public:
		ParticleSystem();

		void Reset();
		void Update(Timestep ts);

		void SetParticles(ParticleData* dataBuffer, ParticleSpecification* specsBuffer);
		void SetParticles(ParticleData* dataBuffer, ParticleSpecification* specsBuffer, uint32_t offsetParticles, uint32_t countParticles);

		const Ref<VertexArray> GetVertexArray() const { return m_VertexArray; }
		ParticleConfiguration& GetConfiguration() { return m_Config; }
	private:
		ParticleConfiguration m_Config;
		float m_EmittedParticles;
		float m_PlayTime;

		Ref<VertexArray>	m_VertexArray;
		Ref<IndirectBuffer> m_IndirectBuffer;

		Ref<ShaderStorageBuffer> m_DataStorage;
		Ref<ShaderStorageBuffer> m_SpecsStorage;
		Ref<AtomicCounter>		 m_DeadCounter;
	};
}