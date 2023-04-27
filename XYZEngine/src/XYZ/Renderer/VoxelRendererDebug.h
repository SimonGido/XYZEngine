#pragma once
#include "Renderer2D.h"
#include "VoxelRenderer.h"

namespace XYZ{
	
	class VoxelRendererDebug
	{
	public:
		VoxelRendererDebug();

		void BeginScene(const VoxelRendererCamera& camera);
		void EndScene();

		void SetViewportSize(uint32_t width, uint32_t height);

	private:
		void createRenderPass();

	private:
		Ref<Renderer2D> m_Renderer;
		Ref<RenderPass> m_GeometryRenderPass;
		Ref<PrimaryRenderCommandBuffer> m_CommandBuffer;

		VoxelRendererCamera m_VoxelCamera;
	};

}
