#pragma once
#include "XYZ/Renderer/RendererCommand.h"
#include "XYZ/Core/Ref.h"
#include "XYZ/Core/Timestep.h"
#include "ParticleModule.h"
#include "ParticleCPU.h"

#include <glm/glm.hpp>

#include <mutex>

namespace XYZ {

	enum class RenderMode
	{
		Billboard,
		Mesh
	};

	struct ParticleRendererCPU : public RendererCommand
	{
		ParticleRendererCPU(uint32_t maxParticles);

		virtual void Bind() const override;

		RenderMode					    Mode;
	private:
		Ref<VertexArray>			    VAO;
		Ref<VertexBuffer>			    InstanceVBO;			
		uint32_t						InstanceCount;	

		friend class ParticleSystemCPU;
		friend class ParticleSystemCPUTest;
	};

	class ParticleSystemCPU : public RefCount
	{
	public:
		ParticleSystemCPU(uint32_t maxParticles);
		~ParticleSystemCPU();

		void SetMaxParticles(uint32_t maxParticles);
		void Play();

		void Update(Timestep ts, const glm::mat4& transform);

		uint32_t GetMaxParticles() const { return m_MaxParticles; }

		void SetEmissionModule(const EmissionModule& module);
		const EmissionModule& GetEmissionModule() const { return m_EmissionModule; }

		ParticleRendererCPU& GetRenderer() { return m_Renderer; }
	private:
		static void emitt(ParticleThreadPass& pass);
		
		void attemptSync();


	private:	
		ParticleThreadPass		m_ThreadPass;
		ParticleRendererCPU		m_Renderer;

		EmissionModule			m_EmissionModule;
		VelocityOverLifeModule  m_VelocityModule;
		SizeOverLifeModule      m_SizeModule;
		uint32_t			    m_MaxParticles;
		
		float					m_Timestep;
		bool					m_Playing;
		bool					m_ProcessByThread;
		
	};
}