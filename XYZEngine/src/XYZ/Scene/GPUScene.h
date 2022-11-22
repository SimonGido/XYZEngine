#pragma once
#include "XYZ/Core/Timestep.h"

#include "XYZ/Particle/GPU/ParticleSystemGPU.h"

#include "XYZ/Renderer/Mesh.h"
#include "XYZ/Asset/Asset.h"

namespace XYZ {


	struct XYZ_API GPUSceneQueue
	{
		struct IndirectDrawCommand
		{
			Ref<StorageBufferAllocation> IndirectCommandSubAllocation;
			Ref<MaterialAsset> RenderMaterial;
			Ref<Mesh> Mesh;
			glm::mat4 Transform;
		};

		struct CommandData
		{
			static constexpr uint32_t sc_TransformCount = 1024;

			uint32_t  CommandCount;
			Padding<12> Padding;
			glm::mat4 Transform[sc_TransformCount];

			uint32_t ReadSize() const
			{
				return sizeof(CommandCount) + sizeof(Padding) + (CommandCount * sizeof(glm::mat4));
			}
		};

		struct ParticleSystemCommand
		{
			Ref<ParticleSystemGPU>					System;
			Ref<StorageBufferAllocation>			IndirectCommandAllocation;
			Ref<StorageBufferAllocation>			CommandDataAllocation;
			
			CommandData								PerCommandData;
			std::vector<IndirectIndexedDrawCommand> IndirectDrawCommands;
			std::vector<IndirectDrawCommand>		DrawCommands;
		};


		std::map<AssetHandle, ParticleSystemCommand> ParticleUpdateCommands;

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

		void prepareCommands(Ref<Scene>& scene, Ref<SceneRenderer>& sceneRenderer);
		void submitParticleCommands(Ref<Scene>& scene, Ref<SceneRenderer>& sceneRenderer);
	private:
		float    m_FrameTimestep; // It can be updated only once per FramesInFlight
		uint32_t m_FrameCounter;
		GPUSceneQueue m_Queue;

	};
}