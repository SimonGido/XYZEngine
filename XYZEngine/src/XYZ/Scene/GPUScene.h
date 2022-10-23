#pragma once
#include "XYZ/Core/Timestep.h"

namespace XYZ {
	class SceneRenderer;
	class Scene;
	class GPUScene
	{
	public:
		GPUScene();

		void OnUpdate(Timestep ts);

		void OnRender(Ref<Scene> scene, Ref<SceneRenderer> sceneRenderer);

	private:
		void updateParticleGPUStorage(Ref<Scene>& scene, Ref<SceneRenderer>& sceneRenderer);
		void submitParticleGPUView(Ref<Scene>& scene, Ref<SceneRenderer>& sceneRenderer);

	private:
		float    m_FrameTimestep; // It can be updated only once per FramesInFlight
		uint32_t m_FrameCounter;
	};
}