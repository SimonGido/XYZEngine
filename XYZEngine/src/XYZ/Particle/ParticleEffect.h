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
		int   IsAlive				= 1;
	};


	struct ParticleLayoutConfiguration
	{
		ParticleLayoutConfiguration();
		ParticleLayoutConfiguration(
			const BufferLayout& particleVertexLayout,
			const BufferLayout& particleInformationlayout
		);
		
		BufferLayout ParticleVertexLayout;
		BufferLayout ParticleInformationLayout; // Per instance
	};

	struct ParticleEffectConfiguration
	{
		ParticleEffectConfiguration(
			uint32_t maxParticles,
			float rate
		);
		
		uint32_t MaxParticles;
		float Rate;
	};

	class ParticleEffect : public RefCount
	{
	public:
		ParticleEffect(const ParticleEffectConfiguration& config, const ParticleLayoutConfiguration& layout);
		~ParticleEffect();

		void Restart();
		void Update(Timestep ts);
		void SetParticles(void* vertexBuffer, void* particleInfo);
		void SetParticlesRange(void* vertexBuffer, void* particleInfo, uint32_t offset, uint32_t count);

		void GetParticles(void* vertexBuffer, void* particleInfo);
		void GetParticlesRange(void* vertexBuffer, void* particleInfo, uint32_t offset, uint32_t count);

		void SetLayout(const ParticleLayoutConfiguration& config);
		void SetConfiguration(const ParticleEffectConfiguration& config);

		float GetPlayTime() const { return m_PlayTime; }
		float GetEmittedParticles() const { return m_EmittedParticles; }

		const Ref<VertexArray> GetVertexArray() const { return m_VertexArray; }
		const Ref<IndirectBuffer> GetIndirectBuffer() const { return m_IndirectBuffer; }
		const Ref<ShaderStorageBuffer> GetShaderStorage() const { return m_VertexStorage; }

		const ParticleEffectConfiguration& GetConfiguration() const { return m_Config; }
		const ParticleLayoutConfiguration& GetLayout() const { return m_Layout; }
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

		ParticleEffectConfiguration m_Config;
		ParticleLayoutConfiguration m_Layout;

		ByteBuffer m_DefaultVertexData;
		ByteBuffer m_DefaultInformationData;

		float m_EmittedParticles = 0.0f;
		float m_PlayTime = 0.0f;

		static constexpr size_t sc_MaxParticlesPerEffect = 10000;
	};



}