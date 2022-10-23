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
		updateParticleGPUStorage(scene, sceneRenderer);
		submitParticleGPUView(scene, sceneRenderer);
		m_FrameCounter++;
	}


	void GPUScene::updateParticleGPUStorage(Ref<Scene>& scene, Ref<SceneRenderer>& sceneRenderer)
	{
		auto& particleGPUStorage = scene->GetRegistry().storage<ParticleComponentGPU>();

		if (m_FrameCounter == 0)
		{
			// This should be done per system, not per component
			for (auto& particleComponent : particleGPUStorage)
			{
				// These allocations should be owned by system, not by component
				sceneRenderer->CreateComputeAllocation(particleComponent.System->GetMaxParticles() * sizeof(ParticleGPU), 0, particleComponent.UpdateAllocation);
				sceneRenderer->CreateComputeAllocation(particleComponent.System->GetMaxParticles() * sizeof(ParticlePropertyGPU), 1, particleComponent.PropertiesAllocation);

				uint32_t emitted = particleComponent.System->Update(m_FrameTimestep * Renderer::GetConfiguration().FramesInFlight);
				uint32_t offset = (particleComponent.System->GetEmittedParticles() - emitted) * particleComponent.System->GetStride();

				if (emitted != 0)
				{
					sceneRenderer->SubmitComputeData(
						&particleComponent.System->GetParticleBuffer().GetData()[offset],
						emitted * particleComponent.System->GetStride(),
						particleComponent.System->GetEmittedParticles() * particleComponent.System->GetStride(),
						particleComponent.PropertiesAllocation
					);

					// TODO: submit compute command with update, it should not be submitted byt submit mesh indirect
				}
			}
		}
	}

	void GPUScene::submitParticleGPUView(Ref<Scene>& scene, Ref<SceneRenderer>& sceneRenderer)
	{
		auto particleGPUView = scene->GetRegistry().view<TransformComponent, ParticleComponentGPU>();

		for (auto entity : particleGPUView)
		{
			auto& [transformComponent, particleComponent] = particleGPUView.get(entity);
			if (particleComponent.System->GetEmittedParticles() != 0)
			{
				// TODO: submit mesh indirect should not call compute command, it should only read data
				sceneRenderer->SubmitMeshIndirect(
					// Rendering data
					particleComponent.Mesh,
					particleComponent.RenderMaterial,
					nullptr,
					transformComponent->WorldTransform,
					// Compute data
					particleComponent.UpdateComputeMaterial,
					particleComponent.PropertiesAllocation,
					particleComponent.UpdateAllocation,

					PushConstBuffer{
						m_FrameTimestep,
						particleComponent.Speed,
						particleComponent.System->GetEmittedParticles(),
						(uint32_t)particleComponent.Loop
					}
				);
			}
		}
	}
}