#include "stdafx.h"
#include "GPUScene.h"

#include "Scene.h"
#include "XYZ/Renderer/SceneRenderer.h"

#include "XYZ/Debug/Profiler.h"

namespace XYZ {

	static GPUSceneQueue::TransformData Mat4ToTransformData(const glm::mat4& transform)
	{
		GPUSceneQueue::TransformData data;
		data.TransformRow[0] = { transform[0][0], transform[1][0], transform[2][0], transform[3][0] };
		data.TransformRow[1] = { transform[0][1], transform[1][1], transform[2][1], transform[3][1] };
		data.TransformRow[2] = { transform[0][2], transform[1][2], transform[2][2], transform[3][2] };
		return data;
	}

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
		cacheAllocations(); // Cache allocations from previous frame
		m_Queue.Clear(); // Clear queue once we cached allocations

		auto particleGPUView = scene->GetRegistry().view<TransformComponent, ParticleComponentGPU>();

		for (auto entity : particleGPUView)
		{
			auto& [transformComponent, particleComponent] = particleGPUView.get(entity);
			
			// Create compute command per material
			auto& command = m_Queue.ParticleCommands[particleComponent.UpdateMaterial->GetHandle()];
			command.UpdateMaterial = particleComponent.UpdateMaterial;
			
			// Create particle data per system
			auto& data = m_Queue.ParticleData[particleComponent.UpdateMaterial->GetHandle()];
			data.System = particleComponent.System;

			// Store handle, so compute command knows on which data operate
			command.ParticleDataHandles.emplace(particleComponent.System->GetHandle());

			// Store per draw command data
			auto& transform = command.PerCommandData.Transform[command.PerCommandData.CommandCount++];
			transform = Mat4ToTransformData(transformComponent->WorldTransform);

			
			auto& drawCommand = command.DrawCommands.emplace_back();
			drawCommand.RenderMaterial = particleComponent.RenderMaterial;
			drawCommand.Mesh = particleComponent.Mesh;
			drawCommand.Transform = transformComponent->WorldTransform;
		}

		// Create allocations per particle system
		for (auto& [handle, data] : m_Queue.ParticleData)
		{
			// Try to reuse allocation cache
			storeParticleDataAllocationsCache(data);

			// It might be required to resize cached allocations
			createParticleDataAllocations(data, sceneRenderer);
		}


		// Create allocations per compute command
		for (auto &[handle, command] : m_Queue.ParticleCommands)
		{
			// Try to reuse allocation cache
			storeParticleCommandAllocationsCache(command);

			// It might be required to resize cached allocations
			createParticleCommandAllocations(command, sceneRenderer);
			
			// Create allocations per draw command
			uint32_t offset = 0;
			for (auto& drawCommand : command.DrawCommands)
			{				
				drawCommand.IndirectCommandSubAllocation = command.IndirectCommandAllocation.CreateSubAllocation(offset, sizeof(IndirectIndexedDrawCommand));
			
				auto& indirectCommand = command.IndirectDrawCommands.emplace_back();
				indirectCommand.Count = drawCommand.Mesh->GetIndexBuffer()->GetCount();

				offset += sizeof(IndirectIndexedDrawCommand);
			}
		}
	}

	void GPUScene::submitParticleCommands(Ref<Scene>& scene, Ref<SceneRenderer>& sceneRenderer)
	{
		XYZ_PROFILE_FUNC("GPUScene::submitParticleCommands");

		if (m_FrameCounter == 0)
		{
			// Submit emission for each system
			submitParticleEmission(sceneRenderer);
		}

		for (auto& [handle, cmd] : m_Queue.ParticleCommands)
		{		
			submitParticleCommandCompute(cmd, sceneRenderer);

			for (auto& dataHandle : cmd.ParticleDataHandles)
			{
				auto& data = m_Queue.ParticleData[dataHandle];
				for (auto& drawCommand : cmd.DrawCommands)
				{
					sceneRenderer->SubmitMeshIndirect(
						// Rendering data
						drawCommand.Mesh,
						drawCommand.RenderMaterial,
						nullptr,
						drawCommand.Transform,
						data.ParticlesResultAllocation,
						drawCommand.IndirectCommandSubAllocation
					);
				}
			}
		}
	}

	void GPUScene::submitParticleEmission(Ref<SceneRenderer>& sceneRenderer)
	{
		for (auto& [key, data] : m_Queue.ParticleData)
		{
			EmissionResult emission = data.System->LastEmission();

			if (emission.EmittedDataSize != 0)
			{
				// Submit emitted data
				sceneRenderer->SubmitComputeData(
					emission.EmittedData,
					emission.EmittedDataSize,
					emission.DataOffset,
					data.ParticlePropertiesAllocation,
					true
				);
			}
		}
	}

	void GPUScene::submitParticleCommandCompute(GPUSceneQueue::ParticleSystemCommand& command, Ref<SceneRenderer>& sceneRenderer)
	{
		// Perform compute per each system we have
		for (auto& dataHandle : command.ParticleDataHandles)
		{
			auto& data = m_Queue.ParticleData[dataHandle];

			std::array<ComputeData, NumTypes> computeData;
			computeData[DataType::Properties].Allocation = data.ParticlePropertiesAllocation;
			computeData[DataType::Result].Allocation = data.ParticlesResultAllocation;

			computeData[DataType::CommandData].Allocation = command.CommandDataAllocation;
			computeData[DataType::CommandData].Data = (std::byte*)&command.PerCommandData;
			computeData[DataType::CommandData].DataSize = command.PerCommandData.ReadSize();

			computeData[DataType::IndirectCommand].Allocation = command.IndirectCommandAllocation;
			computeData[DataType::IndirectCommand].Data = (std::byte*)command.IndirectDrawCommands.data();
			computeData[DataType::IndirectCommand].DataSize = command.IndirectDrawCommands.size() * sizeof(IndirectIndexedDrawCommand);


			sceneRenderer->SubmitCompute(
				command.UpdateMaterial,
				computeData.data(), computeData.size(),
				PushConstBuffer{
					m_FrameTimestep,
					data.System->Speed,
					data.System->GetEmittedParticles(),
					(uint32_t)data.System->Loop
				}
			);
		}
	}

	void GPUScene::createParticleDataAllocations(GPUSceneQueue::ParticleSystemData& data, Ref<SceneRenderer>& sceneRenderer)
	{
		const uint32_t particleResultSize = data.System->GetOutputSize();
		const uint32_t particleStateSize  = data.System->GetInputSize();

		sceneRenderer->CreateComputeAllocation(particleResultSize, 0, data.ParticlesResultAllocation);
		sceneRenderer->CreateComputeAllocation(particleStateSize, 1, data.ParticlePropertiesAllocation);
	}

	void GPUScene::createParticleCommandAllocations(GPUSceneQueue::ParticleSystemCommand& command, Ref<SceneRenderer>& sceneRenderer)
	{
		sceneRenderer->CreateComputeAllocation(command.PerCommandData.ReadSize(), 2, command.CommandDataAllocation);
		sceneRenderer->AllocateIndirectCommand(command.DrawCommands.size(), command.IndirectCommandAllocation);
	}

	void GPUScene::storeParticleDataAllocationsCache(GPUSceneQueue::ParticleSystemData& data)
	{
		auto allocationCacheIt = m_ParticleDataAllocationCache.find(data.System->GetHandle());
		if (allocationCacheIt != m_ParticleDataAllocationCache.end())
		{
			auto& allocation = allocationCacheIt->second;
			data.ParticlePropertiesAllocation = allocation.ParticlePropertiesAllocation;
			data.ParticlesResultAllocation = allocation.ParticlesResultAllocation;
		}
	}

	void GPUScene::storeParticleCommandAllocationsCache(GPUSceneQueue::ParticleSystemCommand& command)
	{
		auto allocationCacheIt = m_ParticleCommandAllocationCache.find(command.UpdateMaterial->GetHandle());
		if (allocationCacheIt != m_ParticleCommandAllocationCache.end())
		{
			auto& allocation = allocationCacheIt->second;
			command.IndirectCommandAllocation = allocation.IndirectCommandAllocation;
			command.CommandDataAllocation = allocation.CommandDataAllocation;
		}
	}


	void GPUScene::cacheAllocations()
	{
		m_ParticleDataAllocationCache.clear();
		m_ParticleCommandAllocationCache.clear();

		for (auto& [handle, cmd] : m_Queue.ParticleCommands)
		{
			auto& allocation = m_ParticleCommandAllocationCache[handle];
			
			allocation.IndirectCommandAllocation = cmd.IndirectCommandAllocation;
			allocation.CommandDataAllocation = cmd.CommandDataAllocation;
		}
		for (auto& [handle, data] : m_Queue.ParticleData)
		{
			auto& allocation = m_ParticleDataAllocationCache[handle];

			allocation.ParticlePropertiesAllocation = data.ParticlePropertiesAllocation;
			allocation.ParticlesResultAllocation = data.ParticlesResultAllocation;
		}
	}
	

	void GPUSceneQueue::Clear()
	{
		ParticleCommands.clear();
		ParticleData.clear();
	}
}