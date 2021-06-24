#pragma once
#include "XYZ/Renderer/RendererCommand.h"
#include "XYZ/Core/Ref.h"
#include "XYZ/Core/Timestep.h"
#include "ParticleCPU.h"

#include <glm/glm.hpp>


namespace XYZ {
	struct ParticleRenderData
	{
		glm::vec4 Color;
		glm::vec4 TexCoord;
		glm::vec2 Position;
		glm::vec2 Size;
		float     Angle;
	};

	struct Emission
	{
		float RateOverTime = 500.0f;
	};

	enum class RenderMode
	{
		Billboard,
		Mesh
	};

	struct ParticleRendererCPU : public RendererCommand
	{
		ParticleRendererCPU(uint32_t maxParticles);

		virtual void Bind() override;

		Ref<VertexArray>			    VAO;
		Ref<VertexBuffer>			    InstanceVBO;
		RenderMode					    Mode;	
		uint32_t					    InstanceCount;
		std::vector<ParticleRenderData> RenderData;
	};

	class ParticleSystemCPU : public RefCount
	{
	public:
		ParticleSystemCPU(uint32_t maxParticles);
		~ParticleSystemCPU();

		void SetMaxParticles(uint32_t maxParticles);

		void Update(Timestep ts);
		
		ParticleRendererCPU m_Renderer;
		Emission		    m_Emission;
	private:
		void emitt(uint32_t count);

	private:
		std::vector<ParticleCPU> m_ParticlePool;	
								 
		uint32_t				 m_MaxParticles;
		uint32_t				 m_ParticlesAlive;
		float					 m_EmittedParticles;
	};
}