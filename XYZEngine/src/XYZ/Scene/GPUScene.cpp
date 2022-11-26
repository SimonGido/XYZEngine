#include "stdafx.h"
#include "GPUScene.h"

#include "Scene.h"
#include "XYZ/Renderer/SceneRenderer.h"

namespace XYZ {

	GPUScene::GPUScene()
		:
		m_FrameTimestep(0.0f),
		m_FrameCounter(0)
	{

	}
	void GPUScene::OnUpdate(Timestep ts)
	{
		if (m_FrameCounter == Renderer::GetConfiguration().FramesInFlight)
		{
			m_FrameTimestep = ts;
			m_FrameCounter = 0;
		}
	}

	void GPUScene::OnRender(Ref<Scene> scene, Ref<SceneRenderer> sceneRenderer)
	{
		XYZ_PROFILE_FUNC("GPUScene::OnRender");
		prepareCommands(scene, sceneRenderer);
		submitParticleCommands(scene, sceneRenderer);
		
		m_FrameCounter++;
	}

	void GPUScene::prepareCommands(Ref<Scene>& scene, Ref<SceneRenderer>& sceneRenderer)
	{
		XYZ_PROFILE_FUNC("GPUScene::prepareCommands");
		cacheAllocations();
		m_Queue.Clear();

		auto particleGPUView = scene->GetRegistry().view<TransformComponent, ParticleComponentGPU>();

		for (auto entity : particleGPUView)
		{
			auto& [transformComponent, particleComponent] = particleGPUView.get(entity);
			auto& command = m_Queue.ParticleCommands[particleComponent.System->GetHandle()];
			command.System = particleComponent.System;
			auto& transform = command.PerCommandData.Transform[command.PerCommandData.CommandCount++];
			transform = transformComponent->WorldTransform;
			
			storeParticleAllocationsCache(command);

			auto& drawCommand = command.DrawCommands.emplace_back();
			drawCommand.RenderMaterial = particleComponent.RenderMaterial;
			drawCommand.Mesh = particleComponent.Mesh;
			drawCommand.Transform = transformComponent->WorldTransform;
		}
		for (auto &[handle, command] : m_Queue.ParticleCommands)
		{
			createParticleCommandAllocations(command, sceneRenderer);
			
			uint32_t offset = 0;
			for (auto& drawCommand : command.DrawCommands)
			{
				drawCommand.IndirectCommandSubAllocation = command.IndirectCommandAllocation->CreateSubAllocation(offset, sizeof(IndirectIndexedDrawCommand));
			
				auto& indirectCommand = command.IndirectDrawCommands.emplace_back();
				indirectCommand.Count = drawCommand.Mesh->GetIndexBuffer()->GetCount();

				offset += sizeof(IndirectIndexedDrawCommand);
			}
		}
	}

	void GPUScene::submitParticleCommands(Ref<Scene>& scene, Ref<SceneRenderer>& sceneRenderer)
	{
		XYZ_PROFILE_FUNC("GPUScene::submitParticleCommands");
		for (auto& [handle, cmd] : m_Queue.ParticleCommands)
		{		
			if (m_FrameCounter == 0)
			{
				submitParticleCommandEmission(cmd, sceneRenderer);
			}

			submitParticleCommandCompute(cmd, sceneRenderer);

			for (auto& drawCommand : cmd.DrawCommands)
			{
				sceneRenderer->SubmitMeshIndirect(
					// Rendering data
					drawCommand.Mesh,
					drawCommand.RenderMaterial,
					nullptr,
					drawCommand.Transform,
					cmd.ParticlesResultAllocation,
					drawCommand.IndirectCommandSubAllocation
				);
			}
		}
	}

	void GPUScene::submitParticleCommandEmission(GPUSceneQueue::ParticleSystemCommand& command, Ref<SceneRenderer>& sceneRenderer)
	{
		EmissionResult emission = command.System->LastEmission();

		if (emission.EmittedDataSize != 0)
		{
			// Submit emitted data
			sceneRenderer->SubmitComputeData(
				emission.EmittedData,
				emission.EmittedDataSize,
				emission.DataOffset,
				command.ParticlePropertiesAllocation,
				true
			);
		}
	}

	void GPUScene::submitParticleCommandCompute(GPUSceneQueue::ParticleSystemCommand& command, Ref<SceneRenderer>& sceneRenderer)
	{
		std::array<ComputeData, NumTypes> computeData;
		computeData[DataType::Properties].Allocation = command.ParticlePropertiesAllocation;
		computeData[DataType::Result].Allocation = command.ParticlesResultAllocation;

		computeData[DataType::CommandData].Allocation = command.CommandDataAllocation;
		computeData[DataType::CommandData].Data = (std::byte*)&command.PerCommandData;
		computeData[DataType::CommandData].DataSize = command.PerCommandData.ReadSize();

		computeData[DataType::IndirectCommand].Allocation = command.IndirectCommandAllocation;
		computeData[DataType::IndirectCommand].Data = (std::byte*)command.IndirectDrawCommands.data();
		computeData[DataType::IndirectCommand].DataSize = command.IndirectDrawCommands.size() * sizeof(IndirectIndexedDrawCommand);


		sceneRenderer->SubmitCompute(
			command.System->ParticleUpdateMaterial,
			computeData.data(), computeData.size(),
			PushConstBuffer{
				m_FrameTimestep,
				command.System->Speed,
				command.System->GetEmittedParticles(),
				(uint32_t)command.System->Loop
			}
		);
	}

	void GPUScene::createParticleCommandAllocations(GPUSceneQueue::ParticleSystemCommand& command, Ref<SceneRenderer>& sceneRenderer)
	{
		const uint32_t particleResultSize = command.System->GetOutputSize();
		const uint32_t particleStateSize = command.System->GetInputSize();

		sceneRenderer->CreateComputeAllocation(particleResultSize, 0, command.ParticlesResultAllocation);
		sceneRenderer->CreateComputeAllocation(particleStateSize, 1, command.ParticlePropertiesAllocation);
		sceneRenderer->CreateComputeAllocation(command.PerCommandData.ReadSize(), 2, command.CommandDataAllocation);
		sceneRenderer->AllocateIndirectCommand(command.DrawCommands.size(), command.IndirectCommandAllocation);

	}

	void GPUScene::storeParticleAllocationsCache(GPUSceneQueue::ParticleSystemCommand& command)
	{
		auto allocationCacheIt = m_AllocationCache.find(command.System->GetHandle());
		if (allocationCacheIt != m_AllocationCache.end())
		{
			auto& allocation = allocationCacheIt->second;
			command.ParticlePropertiesAllocation = allocation.ParticlePropertiesAllocation;
			command.ParticlesResultAllocation = allocation.ParticlesResultAllocation;
			command.IndirectCommandAllocation = allocation.IndirectCommandAllocation;
			command.CommandDataAllocation = allocation.CommandDataAllocation;
		}
	}


	void GPUScene::cacheAllocations()
	{
		m_AllocationCache.clear();
		for (auto& [handle, cmd] : m_Queue.ParticleCommands)
		{
			auto& allocation = m_AllocationCache[handle];
			allocation.ParticlePropertiesAllocation = cmd.ParticlePropertiesAllocation;
			allocation.ParticlesResultAllocation = cmd.ParticlesResultAllocation;
			allocation.IndirectCommandAllocation = cmd.IndirectCommandAllocation;
			allocation.CommandDataAllocation = cmd.CommandDataAllocation;
		}
	}
	

	void GPUSceneQueue::Clear()
	{
		ParticleCommands.clear();
	}
}