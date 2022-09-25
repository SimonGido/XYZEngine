#pragma once
#include "XYZ/Renderer/Renderer.h"
#include "XYZ/Debug/Profiler.h"

namespace XYZ {
	struct LightCullingPassConfiguration
	{
		Ref<StorageBufferSet> LightsBufferSet;
		Ref<UniformBufferSet> CameraBufferSet;
	};
	class LightCullingPass
	{
	public:
		void Init(const LightCullingPassConfiguration& config);

		void Submit(
			const Ref<RenderCommandBuffer>& commandBuffer,
			const Ref<Image2D>& preDepthImage,
			const glm::ivec3& workGroups,
			glm::ivec2 screenSize
		);

	private:
		Ref<PipelineCompute>  m_Pipeline;
		Ref<Material>		  m_Material;
		Ref<MaterialInstance> m_MaterialInstance;

		Ref<StorageBufferSet> m_LightsBufferSet;
		Ref<UniformBufferSet> m_CameraBufferSet;
	};
}