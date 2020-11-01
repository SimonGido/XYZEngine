#pragma once
#include "XYZ/Renderer/VertexArray.h"
#include "XYZ/Renderer/Material.h"
#include "XYZ/Core/Ref.h"
#include "XYZ/Core/Timestep.h"

#include <glm/glm.hpp>
#include <set>
#include <vector>

namespace XYZ {
	
	struct ParticleVertex
	{
		glm::vec4 Color			 = glm::vec4(0);
		glm::vec4 Position		 = glm::vec4(0);
		glm::vec2 TexCoordOffset = glm::vec2(0);
		glm::vec2 Size		     = glm::vec2(0);
		float	  Rotation		 = 0.0f;

	private:
		float	  Alignment		 = 0.0f;
		float	  Alignment2	 = 0.0f;
		float	  Alignment3	 = 0.0f;
	};
	struct ParticleInformation
	{
		glm::vec4 ColorBegin		= glm::vec4(0);
		glm::vec4 ColorEnd			= glm::vec4(0);
		glm::vec2 Velocity			= glm::vec2(0);
		glm::vec2 DefaultVelocity	= glm::vec2(0);
		glm::vec2 DefaultPosition	= glm::vec2(0);

		float SizeBegin				= 0.0f;
		float SizeEnd				= 0.0f;
		float Rotation				= 0.0f;
		float LifeTime				= 0.0f;
	private:
		float TimeAlive 			= 0.0f;
		int IsAlive					= 1;
	};


	struct ParticleEffectConfiguration
	{
		uint32_t Rate = 2;
		uint32_t MaxParticles = 500;
		
		bool	 Loop = true;
	};

	class ParticleEffect2D : public RefCount
	{
	public:
		ParticleEffect2D(const ParticleEffectConfiguration& config);	
		~ParticleEffect2D();

		void Update(Timestep ts);
		void SetParticles(ParticleVertex* vertexBuffer,ParticleInformation* particleInfo);
		void SetParticlesRange(ParticleVertex* vertexBuffer, ParticleInformation* particleInfo, uint32_t offset, uint32_t count);
		void SetConfiguration(const ParticleEffectConfiguration& config) { m_Configuration = config; }

		void SetParticlesRangeTest(void* vertexBuffer, void* particleInfo, uint32_t offset, uint32_t count);

		void GetParticles(ParticleVertex* vertexBuffer, ParticleInformation* particleInfo);
		void GetParticlesRange(ParticleVertex* vertexBuffer, ParticleInformation* particleInfo, uint32_t offset, uint32_t count);	
	
		float GetPlayTime() const { return m_PlayTime; }
		float GetEmittedParticles() const { return m_EmittedParticles; }

		const Ref<VertexArray> GetVertexArray() const { return m_VertexArray; }
		const Ref<IndirectBuffer> GetIndirectBuffer() const { return m_IndirectBuffer; }
		const Ref<ShaderStorageBuffer> GetShaderStorage() const { return m_VertexStorage; }

		const ParticleEffectConfiguration& GetConfiguration() const { return m_Configuration; }
	private:
		enum Bindings
		{
			VERTEX_BINDING,
			PROPS_BINDING,
			INDIRECT_BINDING,
			COUNTER_BINDING,
			NUM_BINDINGS
		};

	private:
		Ref<VertexArray> m_VertexArray;
		Ref<IndirectBuffer> m_IndirectBuffer;

		Ref<ShaderStorageBuffer> m_VertexStorage;
		Ref<ShaderStorageBuffer> m_PropsStorage;
		Ref<AtomicCounter> m_Counter;



		std::vector<ParticleVertex> m_Vertices;
		std::vector<ParticleInformation> m_Data;
	
		ParticleEffectConfiguration m_Configuration;
		float m_EmittedParticles = 0.0f;
		float m_PlayTime = 0.0f;

		static constexpr size_t sc_MaxParticlesPerEffect = 10000;
	};



}