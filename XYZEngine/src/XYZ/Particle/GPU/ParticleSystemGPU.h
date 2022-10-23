#pragma once
#include "XYZ/Core/Timestep.h"

#include "ParticleSystemLayout.h"
#include "ParticleEmitterGPU.h"


#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

namespace XYZ {
	
	

	struct ParticleGPU
	{
		// Data that are rendered
		glm::vec4  TransformRow0;
		glm::vec4  TransformRow1;
		glm::vec4  TransformRow2;
		glm::vec4  Color;	
	};

	struct ParticlePropertyGPU
	{
		// Spawn state
		glm::vec4 StartPosition;
		glm::vec4 StartColor;
		glm::quat StartRotation;
		glm::vec4 StartScale;
		glm::vec4 StartVelocity;
	
		// If module enabled, end state
		glm::vec4 EndColor;
		glm::quat EndRotation;
		glm::vec4 EndScale;
		glm::vec4 EndVelocity;

		glm::vec4  Position;
		float	   LifeTime;
		float      LifeReamining;
	private:
		Padding<8> Padding;
	};


	class ParticleBuffer
	{
	public:
		ParticleBuffer() = default;
		ParticleBuffer(uint32_t maxParticles, uint32_t stride);

		void SetMaxParticles(uint32_t maxParticles, uint32_t stride);

		std::byte* GetData() { return m_Data.data(); }
		std::byte* GetData(uint32_t particleOffset);

		const std::byte* GetData() const { return m_Data.data(); }

		uint32_t   GetStride()		 const { return m_Stride; }
		uint32_t   GetMaxParticles() const { return GetBufferSize() / m_Stride; }
		uint32_t   GetBufferSize()   const { return static_cast<uint32_t>(m_Data.size()); }
	private:
		std::vector<std::byte> m_Data;
		uint32_t			   m_Stride = 0;
	};

	class ParticleSystemGPU : public RefCount // TODO: asset
	{
	public:
		ParticleSystemGPU(ParticleSystemLayout layout, uint32_t maxParticles);

		uint32_t Update(Timestep ts);

		const ParticleSystemLayout& GetLayout()			const { return m_Layout; }
		const ParticleBuffer&		GetParticleBuffer() const { return m_ParticleBuffer; }
		
		uint32_t GetStride()		   const { return m_Layout.GetStride(); }
		uint32_t GetEmittedParticles() const { return m_EmittedParticles; }
		uint32_t GetMaxParticles()	   const { return m_ParticleBuffer.GetMaxParticles(); }

		std::vector<ParticleEmitterGPU> ParticleEmitters;
		
	private:
		ParticleSystemLayout m_Layout;
		ParticleBuffer		 m_ParticleBuffer;
		uint32_t			 m_EmittedParticles;
	};

	
}