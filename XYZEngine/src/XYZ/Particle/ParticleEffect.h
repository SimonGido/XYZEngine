#pragma once
#include "XYZ/Renderer/VertexArray.h"
#include "XYZ/Renderer/Material.h"
#include "XYZ/Core/Ref.h"
#include "XYZ/Core/Timestep.h"

#include "ParticleModule.h"
#include "Particle.h"

#include <glm/glm.hpp>

namespace XYZ {
	class ParticleEffect : public RefCount
	{
	public:
		ParticleEffect(uint32_t maxParticles, const Ref<Shader>& computeShader);
		
		void Update(Timestep ts);
		void Compute();
		void Reset();


		void SetBufferSize(const std::string& name, uint32_t size);
		void SetBufferData(const std::string& name, void* data, uint32_t count, uint32_t elementSize, uint32_t offset = 0);

		
		const Ref<Shader>& GetComputeShader() const { return m_ComputeShader; }
		const Ref<VertexArray>& GetVertexArray() const { return m_VertexArray; }
		const Ref<IndirectBuffer>& GetIndirectBuffer() const { return m_DrawCommand; }
	private:
		uint32_t m_MaxParticles;
		float    m_Rate;
		float    m_EmittedParticles;
		float    m_PlayTime;

	private:
		Ref<Shader> m_ComputeShader;
		Ref<VertexArray> m_VertexArray;

		struct Buffer
		{
			std::string				 Name;
			Ref<ShaderStorageBuffer> Storage;
			uint32_t				 ElementSize;
			uint32_t			     ElementCount;
			bool					 RenderBuffer;
			bool				     ParticleBuffer;
		};

		struct Counter
		{
			std::string		   Name;
			Ref<AtomicCounter> Atomic;
		};

		std::vector<Buffer>  m_Buffers;
		std::vector<Counter> m_Counters;
		Ref<IndirectBuffer>  m_DrawCommand;
	};
}