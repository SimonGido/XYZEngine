#include "stdafx.h"
#include "VoxelRendererDebug.h"


namespace XYZ {
	VoxelRendererDebug::VoxelRendererDebug()
	{
		m_CommandBuffer = PrimaryRenderCommandBuffer::Create(0, "VoxelDebugRenderer");
		createRenderPass();
	}
	void VoxelRendererDebug::BeginScene(const VoxelRendererCamera& camera)
	{
		m_VoxelCamera = camera;
	}
	void VoxelRendererDebug::EndScene()
	{
	}
	void VoxelRendererDebug::SetViewportSize(uint32_t width, uint32_t height)
	{
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
}
