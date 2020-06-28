#pragma once
#include "XYZ/ECS/Component.h"
#include "XYZ/Renderer/VertexArray.h"
#include "XYZ/Renderer/Material.h"

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
		int IsAlive					= true;
	};


	
	/* !@class ParticleEffect2D 
	* @brief gpu based particle effect
	*/
	class ParticleEffect2D : public Type<ParticleEffect2D>
	{
	public:
		/**
		* Construct
		* @param[in] maxParticles
		* @param[in] material
		* @param[in] renderMaterial
		*/
		ParticleEffect2D(uint32_t maxParticles, std::shared_ptr<Material> material, std::shared_ptr<Material> renderMaterial);
		
		~ParticleEffect2D();

		/**
		* Rendering function
		*/
		void Render();

		/**
		* Update data in the gpu and compute
		* @param[in] dt
		*/
		void Update(float dt);


		const std::shared_ptr<Material> GetMaterial() { return m_Material; }
		

		/**
		* Set max number of particles
		* @param[in] vertexBuffer
		* @param[in] particleInfo
		*/
		void SetParticles(ParticleVertex* vertexBuffer,ParticleInformation* particleInfo);
		
		/**
		* Set particles in range
		* @param[in] vertexBuffer
		* @param[in] particleInfo
		*/
		void SetParticlesRange(ParticleVertex* vertexBuffer, ParticleInformation* particleInfo, uint32_t offset, uint32_t count);

		/**
		* @param[out] vertexBuffer
		* @param[out] particleInfo
		* @return number of particles
		*/
		uint32_t GetParticles(ParticleVertex* vertexBuffer, ParticleInformation* particleInfo);
		
		/**
		* @param[out] vertexBuffer
		* @param[out] particleInfo
		* @param[in] offset
		* @param[in] count
		* @return number of particles
		*/
		uint32_t GetParticlesRange(ParticleVertex* vertexBuffer, ParticleInformation* particleInfo, uint32_t offset, uint32_t count);
		
		uint32_t GetNumberOfParticles() { return m_MaxParticles; }

	private:

		struct Emitter
		{
			int Rate = 20;
			float EmittedParticles = 0.0f;
			static constexpr float EmitDuration = 1.0f;
		};
		Emitter m_Emitter;
		
		struct Renderable
		{
			std::shared_ptr<VertexArray> VertexArray;
			std::shared_ptr<IndirectBuffer> IndirectBuffer;
			std::shared_ptr<Material> Material;
		};
		Renderable m_Renderable;


		enum
		{
			VERTEX_BINDING,
			PROPS_BINDING,
			INDIRECT_BINDING,
			COUNTER_BINDING,
			NUM_BINDINGS
		};

	private:
		std::shared_ptr<Material> m_Material;
		std::shared_ptr<Shader> m_Shader;

		uint32_t m_MaxParticles = 0;

		std::vector<ParticleVertex> m_Vertices;
		std::vector<ParticleInformation> m_Data;

		std::shared_ptr<ShaderStorageBuffer> m_VertexStorage;
		std::shared_ptr<ShaderStorageBuffer> m_PropsStorage;

		std::shared_ptr<AtomicCounter> m_Counter;

		bool m_Loop = true;
		float m_LifeTime = 3.0f;
		double m_PlayTime = 0.0f;
		static constexpr size_t sc_MaxParticlesPerEffect = 10000;
	};



}