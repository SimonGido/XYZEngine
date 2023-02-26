#pragma once
#include "XYZ/Renderer/Renderer.h"


namespace XYZ {
	struct LightCullingPassConfiguration
	{
		Ref<UniformBufferSet> UniformBufferSet;
		Ref<StorageBufferSet> StorageBufferSet;
	};
	class XYZ_API LightCullingPass
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

		Ref<StorageBufferSet> m_StorageBufferSet;
		Ref<UniformBufferSet> m_UniformBufferSet;
	};
}