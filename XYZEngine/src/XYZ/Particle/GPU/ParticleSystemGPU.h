#pragma once
#include "XYZ/Core/Timestep.h"
#include "XYZ/Asset/Asset.h"
#include "XYZ/Renderer/StorageBufferAllocator.h"
#include "XYZ/Asset/Renderer/MaterialAsset.h"

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
		const std::byte* GetData(uint32_t particleOffset) const;

		uint32_t   GetStride()		 const { return m_Stride; }
		uint32_t   GetMaxParticles() const { return GetBufferSize() / m_Stride; }
		uint32_t   GetBufferSize()   const { return static_cast<uint32_t>(m_Data.size()); }
	
	private:
		std::vector<std::byte> m_Data;
		uint32_t			   m_Stride = 0;
	};

	class ParticleSystemGPU : public Asset
	{
	public:
		ParticleSystemGPU(ParticleSystemLayout inputLayout, ParticleSystemLayout outputLayout, uint32_t maxParticles);

		virtual AssetType GetAssetType() const { return AssetType::ParticleSystemGPU; }
		static  AssetType GetStaticType() { return AssetType::ParticleSystemGPU; }

		uint32_t Update(Timestep ts);

		const ParticleSystemLayout& GetInputLayout()	const { return m_InputLayout; }
		const ParticleSystemLayout& GetOutputLayout()	const { return m_OutputLayout; }
		const ParticleBuffer&		GetParticleBuffer() const { return m_ParticleBuffer; }
		
		uint32_t GetInputSize()  const { return m_ParticleBuffer.GetBufferSize(); }
		uint32_t GetOutputSize() const { return GetMaxParticles() * m_OutputLayout.GetStride(); }

		uint32_t GetInputStride()	   const { return m_InputLayout.GetStride(); }
		uint32_t GetEmittedParticles() const { return m_EmittedParticles; }
		uint32_t GetMaxParticles()	   const { return m_ParticleBuffer.GetMaxParticles(); }

		std::vector<ParticleEmitterGPU> ParticleEmitters;
		
		Ref<MaterialAsset>				ParticleUpdateMaterial;

		Ref<StorageBufferAllocation>	ParticlePropertiesAllocation;
		Ref<StorageBufferAllocation>	ParticlesResultAllocation;

		float    Speed = 1.0f;
		bool	 Loop  = true;
	private:
		ParticleSystemLayout m_InputLayout;
		ParticleSystemLayout m_OutputLayout;

		ParticleBuffer		 m_ParticleBuffer;
		uint32_t			 m_EmittedParticles;
	};

	class ParticleSystemGPUShaderGenerator
	{
		struct Variable
		{
			std::string Type;
			std::string Name;
			uint32_t    Size = 0; // only when adding structs
			bool		IsArray = false;
		};
	public:
		ParticleSystemGPUShaderGenerator(const Ref<ParticleSystemGPU>& particleSystem);

		const std::string& GetSource() const { return m_SourceCode; }
	private:
		void addStruct(const std::string_view name, const std::vector<Variable>& variables);
		void addSSBO(uint32_t binding, const std::string_view name, const std::vector<Variable>& variables);

		void addUniforms(const std::string_view name, const std::string_view declName, const std::vector<Variable>& variables);

		void addEntryPoint(uint32_t groupX, uint32_t groupY, uint32_t groupZ);
		void addUpdate();

		static std::vector<Variable> particleVariablesToVariables(const std::vector<ParticleVariable>& variables);
	private:
		std::string m_SourceCode;
	};
}