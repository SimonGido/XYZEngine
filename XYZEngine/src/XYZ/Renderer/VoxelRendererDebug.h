#pragma once
#include "Renderer2D.h"
#include "VoxelRenderer.h"


namespace XYZ{
	
	class VoxelRendererDebug
	{
	public:
		VoxelRendererDebug(
			Ref<PrimaryRenderCommandBuffer> commandBuffer,
			Ref<UniformBufferSet> uniformBufferSet
		);

		void BeginScene(const VoxelRendererCamera& camera);
		void EndScene(Ref<Image2D> image);

		void SetViewportSize(uint32_t width, uint32_t height);



	private:
		void render();
		void createRenderPass();
		void createPipeline();

	private:
		Ref<PrimaryRenderCommandBuffer> m_CommandBuffer;
		Ref<UniformBufferSet> m_UniformBufferSet;

		Ref<Renderer2D> m_Renderer;
		Ref<RenderPass> m_GeometryRenderPass;


		Ref<Pipeline> m_LinePipeline;
		Ref<Material> m_Material;
		Ref<MaterialInstance> m_MaterialInstance;

		VoxelRendererCamera m_VoxelCamera;
	};

}
