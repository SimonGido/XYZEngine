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
			Ref<StorageBufferAllocation> IndirectCommandSubAllocation;
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

		struct ParticleSystemCommand
		{
			Ref<ParticleSystemGPU>					System;
			
			Ref<StorageBufferAllocation>			ParticlePropertiesAllocation;
			Ref<StorageBufferAllocation>			ParticlesResultAllocation;
			Ref<StorageBufferAllocation>			IndirectCommandAllocation;
			Ref<StorageBufferAllocation>			CommandDataAllocation;
			
			CommandData								PerCommandData;
			std::vector<IndirectIndexedDrawCommand> IndirectDrawCommands;
			std::vector<IndirectDrawCommand>		DrawCommands;
		};


		std::map<AssetHandle, ParticleSystemCommand> ParticleCommands;

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

		void createParticleCommandAllocations(GPUSceneQueue::ParticleSystemCommand& command, Ref<SceneRenderer>& sceneRenderer);
		void storeParticleAllocationsCache(GPUSceneQueue::ParticleSystemCommand& command);
		
		void submitParticleCommandEmission(GPUSceneQueue::ParticleSystemCommand& command, Ref<SceneRenderer>& sceneRenderer);
		void submitParticleCommandCompute(GPUSceneQueue::ParticleSystemCommand& command, Ref<SceneRenderer>& sceneRenderer);
	private:
		float    m_FrameTimestep; // It can be updated only once per FramesInFlight
		uint32_t m_FrameCounter;
		GPUSceneQueue m_Queue;

		struct ParticleAllocation
		{
			Ref<StorageBufferAllocation> ParticlePropertiesAllocation;
			Ref<StorageBufferAllocation> ParticlesResultAllocation;
			Ref<StorageBufferAllocation> IndirectCommandAllocation;
			Ref<StorageBufferAllocation> CommandDataAllocation;
		};

		std::unordered_map<AssetHandle, ParticleAllocation> m_AllocationCache;
	};
}