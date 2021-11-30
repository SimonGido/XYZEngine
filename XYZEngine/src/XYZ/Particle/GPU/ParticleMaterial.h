#pragma once
#include "XYZ/Renderer/VertexArray.h"
#include "XYZ/Renderer/Material.h"
#include "XYZ/Core/Ref.h"
#include "XYZ/Core/Timestep.h"

#include "Particle.h"

#include <glm/glm.hpp>

namespace XYZ {

	class ParticleMaterial : public RefCount
	{
	public:
		ParticleMaterial(uint32_t maxParticles, const Ref<Shader>& computeShader);
		
		void Compute() const;
		void ResetCounters();

		template<typename T>
		void Set(const std::string& name, const T& val);
		
		template <typename T>
		T& Get(const std::string& name);


		void SetMaxParticles(uint32_t maxParticles);
		void SetComputeShader(const Ref<Shader>& computeShader);
		void SetParticleBuffersElementCount(uint32_t count);
		void SetBufferElementCount(const std::string& name, uint32_t count);
		void SetBufferSize(const std::string& name, uint32_t size);
		void SetBufferData(const std::string& name, void* data, uint32_t count, uint32_t elementSize, uint32_t offset = 0);

		void GetBufferData(const std::string& name, void** data, uint32_t count, uint32_t offset = 0);

		const Ref<Shader>& GetComputeShader() const { return m_ComputeShader; }
		const Ref<VertexArray>& GetVertexArray() const { return m_VertexArray; }
		const Ref<IndirectBuffer>& GetIndirectBuffer() const { return m_DrawCommand; }
		
		const uint8_t* GetUniformBuffer() const { return m_UniformBuffer; }
		uint32_t GetMaxParticles() const { return m_MaxParticles; }
	private:
		void parse();
		void rebuild();
		void resize();
		
		const ShaderUniform* findUniform(const std::string& name) const;
	private:
		Ref<Shader>		 m_ComputeShader;
		Ref<VertexArray> m_VertexArray;
		ByteBuffer		 m_UniformBuffer;
		uint32_t		 m_MaxParticles;

		struct Buffer
		{
			std::string				 Name;
			Ref<StorageBuffer> Storage;
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

	template <typename T>
	void ParticleMaterial::Set(const std::string& name, const T& val)
	{
		const auto uni = findUniform(name);
		XYZ_ASSERT(uni, "Particle material uniform does not exist ", name.c_str());
		m_UniformBuffer.Write((unsigned char*)&val, uni->GetSize(), uni->GetOffset());
	}

	template <typename T>
	T& ParticleMaterial::Get(const std::string& name)
	{
		const auto uni = findUniform(name);
		XYZ_ASSERT(uni, "Particle material uniform does not exist ", name.c_str());
		return *(T*)&m_UniformBuffer[uni->GetOffset()];
	}
}