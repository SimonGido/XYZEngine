#pragma once
#include "XYZ/Core/Timestep.h"

#include "XYZ/Particle/GPU/ParticleSystemGPU.h"

#include "XYZ/Renderer/Mesh.h"
#include "XYZ/Asset/Asset.h"

namespace XYZ {


	struct XYZ_API GPUSceneQueue
	{

		struct TransformData
		{
			glm::vec4 TransformRow[3];
		};
		struct IndirectDrawCommand
		{
			StorageBufferAllocation		 IndirectCommandSubAllocation;
			Ref<MaterialAsset>			 RenderMaterial;
			Ref<Mesh>					 Mesh;
			glm::mat4					 Transform;
		};

		struct XYZ_API CommandData
		{
			static constexpr uint32_t sc_TransformCount = 1024;

			uint32_t	  CommandCount;
			Padding<12>   Padding;
			TransformData Transform[sc_TransformCount];

			uint32_t ReadSize() const
			{
				return sizeof(CommandCount) + sizeof(Padding) + (CommandCount * sizeof(TransformData));
			}
		};

		struct XYZ_API ParticleSystemData
		{
			Ref<ParticleSystemGPU>	System;
			StorageBufferAllocation	ParticlePropertiesAllocation;
			StorageBufferAllocation	ParticlesResultAllocation;
		};

		struct ParticleSystemCommand
		{			
			Ref<MaterialAsset>						UpdateMaterial;
			
			StorageBufferAllocation					IndirectCommandAllocation;
			StorageBufferAllocation					CommandDataAllocation;
			
			CommandData								PerCommandData;
			
			std::set<AssetHandle>					  ParticleDataHandles;
			std::vector<IndirectIndexedDrawCommand>   IndirectDrawCommands;
			std::vector<IndirectDrawCommand>		  DrawCommands;
			
		};

		std::map<AssetHandle, ParticleSystemCommand> ParticleCommands;
		std::unordered_map<AssetHandle, ParticleSystemData> ParticleData;

		void Clear();
	};

	
	class SceneRenderer;
	class Scene;
	class XYZ_API GPUScene // TODO: find more suitable name
	{
		enum DataType
		{
			Properties, Result, CommandData, IndirectCommand, NumTypes
		};
	public:
		GPUScene();

		void OnUpdate(Timestep ts);

		void OnRender(Ref<Scene> scene, Ref<SceneRenderer> sceneRenderer);

	private:
		void cacheAllocations();

		void prepareCommands(Ref<Scene>& scene, Ref<SceneRenderer>& sceneRenderer);
		void submitParticleCommands(Ref<Scene>& scene, Ref<SceneRenderer>& sceneRenderer);

		void createParticleDataAllocations(GPUSceneQueue::ParticleSystemData& data, Ref<SceneRenderer>& sceneRenderer);
		void createParticleCommandAllocations(GPUSceneQueue::ParticleSystemCommand& command, Ref<SceneRenderer>& sceneRenderer);
		void storeParticleDataAllocationsCache(GPUSceneQueue::ParticleSystemData& data);
		void storeParticleCommandAllocationsCache(GPUSceneQueue::ParticleSystemCommand& command);

		void submitParticleEmission(Ref<SceneRenderer>& sceneRenderer);
		void submitParticleCommandCompute(GPUSceneQueue::ParticleSystemCommand& command, Ref<SceneRenderer>& sceneRenderer);
	private:
		float    m_FrameTimestep; // It can be updated only once per FramesInFlight
		uint32_t m_FrameCounter;
		GPUSceneQueue m_Queue;

		struct ParticleDataAllocation
		{
			StorageBufferAllocation ParticlePropertiesAllocation;
			StorageBufferAllocation ParticlesResultAllocation;

		};

		struct ParticleCommandAllocation
		{
			StorageBufferAllocation IndirectCommandAllocation;
			StorageBufferAllocation CommandDataAllocation;
		};

		std::unordered_map<AssetHandle, ParticleDataAllocation> m_ParticleDataAllocationCache;
		std::unordered_map<AssetHandle, ParticleCommandAllocation> m_ParticleCommandAllocationCache;
	};
}