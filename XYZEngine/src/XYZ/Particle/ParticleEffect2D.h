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
		ParticleEffectConfiguration(uint32_t maxParticles, uint32_t rate, bool loop);
		ParticleEffectConfiguration(
			const BufferLayout& particleVertexLayout, 
			const BufferLayout& particleInformationlayout,
			uint32_t maxParticles, uint32_t rate, bool loop
		);

		BufferLayout ParticleVertexLayout;	
		BufferLayout ParticleInformationLayout; // Per instance
		uint32_t MaxParticles;
		uint32_t Rate;
		bool	 Loop;
	};

	class ParticleEffect2D : public RefCount
	{
	public:
		ParticleEffect2D(const ParticleEffectConfiguration& config);	
		~ParticleEffect2D();

		void Update(Timestep ts);
		void SetParticles(void* vertexBuffer, void* particleInfo);
		void SetParticlesRange(void* vertexBuffer, void* particleInfo, uint32_t offset, uint32_t count);

		void GetParticles(void* vertexBuffer, void* particleInfo);
		void GetParticlesRange(void* vertexBuffer, void* particleInfo, uint32_t offset, uint32_t count);

		void SetConfiguration(const ParticleEffectConfiguration& config) { XYZ_ASSERT(false, ""); m_Configuration = config; }

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

		ParticleEffectConfiguration m_Configuration;
		float m_EmittedParticles = 0.0f;
		float m_PlayTime = 0.0f;

		static constexpr size_t sc_MaxParticlesPerEffect = 10000;
	};



}