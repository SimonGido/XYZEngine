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
		m_Queue.Clear();

		auto particleGPUView = scene->GetRegistry().view<TransformComponent, ParticleComponentGPU>();

		for (auto entity : particleGPUView)
		{
			auto& [transformComponent, particleComponent] = particleGPUView.get(entity);
			auto& command = m_Queue.ParticleUpdateCommands[particleComponent.System->GetHandle()];
			command.System = particleComponent.System;

			auto& drawCommand = command.DrawCommands.emplace_back();
			drawCommand.RenderMaterial = particleComponent.RenderMaterial;
			drawCommand.Mesh = particleComponent.Mesh;
			drawCommand.Transform = transformComponent->WorldTransform;
		}
		for (auto &[handle, command] : m_Queue.ParticleUpdateCommands)
		{
			sceneRenderer->AllocateIndirectCommand(command.DrawCommands.size(), command.IndirectCommandAllocation);
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
		for (auto& [handle, cmd] : m_Queue.ParticleUpdateCommands)
		{
			const uint32_t particleResultSize = cmd.System->GetOutputSize();
			const uint32_t particleStateSize = cmd.System->GetInputSize();

			sceneRenderer->CreateComputeAllocation(particleResultSize, 0, cmd.System->ParticlesResultAllocation);
			sceneRenderer->CreateComputeAllocation(particleStateSize, 1, cmd.System->ParticlePropertiesAllocation);
			
			if (m_FrameCounter == 0)
			{
				const uint32_t emitted = cmd.System->Update(m_FrameTimestep * Renderer::GetConfiguration().FramesInFlight);
				const uint32_t offsetParticles = (cmd.System->GetEmittedParticles() - emitted);
				const uint32_t dataOffset = offsetParticles * cmd.System->GetInputStride();

				if (emitted != 0)
				{
					// Submit emitted data
					sceneRenderer->SubmitComputeData(
						cmd.System->GetParticleBuffer().GetData(offsetParticles),
						emitted * cmd.System->GetInputStride(),
						dataOffset,
						cmd.System->ParticlePropertiesAllocation,
						true
					);
				}
			}

			std::array<ComputeData, NumTypes> computeData;
			computeData[DataType::Properties].Allocation = cmd.System->ParticlePropertiesAllocation;
			computeData[DataType::Result].Allocation = cmd.System->ParticlesResultAllocation;
			computeData[DataType::IndirectCommand].Allocation = cmd.IndirectCommandAllocation;
			computeData[DataType::IndirectCommand].Data = (std::byte*)cmd.IndirectDrawCommands.data();
			computeData[DataType::IndirectCommand].DataSize = cmd.IndirectDrawCommands.size() * sizeof(IndirectIndexedDrawCommand);


			sceneRenderer->SubmitCompute(
				cmd.System->ParticleUpdateMaterial,
				computeData.data(), computeData.size(),
				PushConstBuffer{
					static_cast<uint32_t>(cmd.IndirectDrawCommands.size()),
					m_FrameTimestep,
					cmd.System->Speed,
					cmd.System->GetEmittedParticles(),
					(uint32_t)cmd.System->Loop
				}
			);

			for (auto& drawCommand : cmd.DrawCommands)
			{
				sceneRenderer->SubmitMeshIndirect(
					// Rendering data
					drawCommand.Mesh,
					drawCommand.RenderMaterial,
					nullptr,
					drawCommand.Transform,
					cmd.System->ParticlesResultAllocation,
					drawCommand.IndirectCommandSubAllocation
				);
			}
		}
	}
	

	void GPUSceneQueue::Clear()
	{
		ParticleUpdateCommands.clear();
	}
}