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
			for (auto& particleComponent : particleGPUStorage)
			{
				sceneRenderer->CreateComputeAllocation(particleComponent.Buffer.GetMaxParticles() * sizeof(ParticleGPU), 0, particleComponent.UpdateAllocation);
				sceneRenderer->CreateComputeAllocation(particleComponent.Buffer.GetMaxParticles() * sizeof(ParticlePropertyGPU), 1, particleComponent.PropertiesAllocation);

				if (particleComponent.EmittedParticles == particleComponent.Buffer.GetMaxParticles())
					continue;

				const uint32_t bufferOffset = particleComponent.EmittedParticles * particleComponent.System->GetStride();
				const uint32_t bufferSize = particleComponent.Buffer.GetBufferSize() - bufferOffset;
				std::byte* particleBuffer = &particleComponent.Buffer.GetData()[bufferOffset];

				uint32_t emitted = particleComponent.System->Update(
					m_FrameTimestep * Renderer::GetConfiguration().FramesInFlight, 
					particleBuffer, 
					bufferSize
				);

				if (emitted != 0)
				{
					sceneRenderer->SubmitComputeData(
						particleBuffer,
						emitted * particleComponent.System->GetStride(),
						particleComponent.EmittedParticles * particleComponent.System->GetStride(),
						particleComponent.PropertiesAllocation
					);
					particleComponent.EmittedParticles += emitted;
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
			if (particleComponent.EmittedParticles != 0)
			{
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
						particleComponent.EmittedParticles,
						(int)particleComponent.Running
					}
				);
			}
		}
	}
}