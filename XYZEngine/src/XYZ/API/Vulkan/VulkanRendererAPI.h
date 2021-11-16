#pragma once
#include "XYZ/Renderer/RendererAPI.h"

namespace XYZ {

	class VulkanRendererAPI : public RendererAPI
	{
	public:
		virtual void Init() override;
		virtual void BeginFrame() override;
		virtual void EndFrame() override;
		virtual void TestDraw(const Ref<RenderPass>& renderPass, const Ref<RenderCommandBuffer>& commandBuffer, const Ref<Pipeline>& pipeline, const Ref<VertexBuffer>& vbo, const Ref<IndexBuffer>& ibo) override;
	};
}