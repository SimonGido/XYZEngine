#include "stdafx.h"
#include "VoxelRendererDebug.h"

#include "Renderer.h"

namespace XYZ {
	VoxelRendererDebug::VoxelRendererDebug(
		Ref<PrimaryRenderCommandBuffer> commandBuffer,
		Ref<UniformBufferSet> uniformBufferSet
	)
		:
		m_CommandBuffer(commandBuffer),
		m_UniformBufferSet(uniformBufferSet)
	{
		createRenderPass();
	}
	void VoxelRendererDebug::BeginScene(const VoxelRendererCamera& camera)
	{
		m_VoxelCamera = camera;
	}
	void VoxelRendererDebug::EndScene(Ref<Image2D> image)
	{
	}
	void VoxelRendererDebug::SetViewportSize(uint32_t width, uint32_t height)
	{
	}
	void VoxelRendererDebug::render()
	{
		Renderer::BeginRenderPass(
			m_CommandBuffer,
			m_GeometryRenderPass,
			false,
			true
		);

		Renderer::BindPipeline(
			m_CommandBuffer,
			m_LinePipeline,
			m_UniformBufferSet,
			nullptr,
			m_Material
		);

		m_Renderer->FlushLines(m_LinePipeline, m_MaterialInstance, true);
		m_Renderer->EndScene();

		Renderer::EndRenderPass(m_CommandBuffer);
	}
	void VoxelRendererDebug::createRenderPass()
	{
		FramebufferSpecification framebufferSpec;
		framebufferSpec.Attachments = {
				FramebufferTextureSpecification(ImageFormat::RGBA32F),
				FramebufferTextureSpecification(ImageFormat::RGBA32F),
				FramebufferTextureSpecification(ImageFormat::DEPTH24STENCIL8)
		};
		framebufferSpec.Samples = 1;
		framebufferSpec.ClearOnLoad = false;
		framebufferSpec.ClearColor = { 0.0f, 0.0f, 0.0f, 0.0f };

		Ref<Framebuffer> framebuffer = Framebuffer::Create(framebufferSpec);

		RenderPassSpecification renderPassSpec;
		renderPassSpec.TargetFramebuffer = framebuffer;
		m_GeometryRenderPass = RenderPass::Create(renderPassSpec);
	}
	void VoxelRendererDebug::createPipeline()
	{
	}
}
