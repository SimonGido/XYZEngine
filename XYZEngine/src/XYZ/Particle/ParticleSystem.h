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

		glm::vec4 StartColor;
		glm::vec4 EndColor;
		glm::vec2 StartSize;
		glm::vec2 EndSize;

		uint32_t  MaxParticles;
		float     Rate;
		float	  Gravity;
		float     Speed;
		float     Time;
		bool	  Repeat;
	};

	class ParticleSystem : public RefCount
	{
	public:
		ParticleSystem(const ParticleConfiguration& config = ParticleConfiguration());

		void Reset();
		void Update(Timestep ts);

		void SetParticles(ParticleData* dataBuffer, ParticleSpecification* specsBuffer);
		void SetParticles(ParticleData* dataBuffer, ParticleSpecification* specsBuffer, uint32_t offsetParticles, uint32_t countParticles);


		const Ref<VertexArray>& GetVertexArray() const { return m_VertexArray; }
		const Ref<ShaderStorageBuffer>& GetShaderStorage() const { return m_DataStorage; }
		const Ref<IndirectBuffer>& GetIndirectBuffer() const { return m_IndirectBuffer; }
		
		ParticleConfiguration& GetConfiguration() { return m_Config; }
		int32_t GetEmittedParticles() const { return (int)std::ceil(m_EmittedParticles); }
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