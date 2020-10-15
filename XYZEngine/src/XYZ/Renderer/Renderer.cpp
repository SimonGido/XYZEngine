#include "stdafx.h"
#include "Renderer.h"

#include "Renderer2D.h"
#include "InGuiRenderer.h"

namespace XYZ {


	struct RendererData
	{
		RenderCommandQueue CommandQueue;
	};

	static RendererData s_Data;

	void Renderer::Init()
	{
		RendererAPI::Init();
		Renderer2D::Init();
		InGuiRenderer::Init();
	}

	void Renderer::Shutdown()
	{
		Renderer2D::Shutdown();
		InGuiRenderer::Shutdown();
	}

	void Renderer::Clear()
	{
		Renderer::Submit([=]() {
			RendererAPI::Clear();
		});
	}

	void Renderer::SetClearColor(const glm::vec4& color)
	{
		Renderer::Submit([=]() {
			RendererAPI::SetClearColor(color);
		});
	}

	void Renderer::SetViewPort(uint32_t x, uint32_t y, uint32_t width, uint32_t height)
	{
		Renderer::Submit([=]() {
			RendererAPI::SetViewport(x, y, width, height);
		});
	}

	void Renderer::DrawIndexed(PrimitiveType type, uint32_t indexCount)
	{
		Renderer::Submit([=]() {
			RendererAPI::DrawIndexed(type, indexCount);
		});
	}

	void Renderer::DrawInstanced(const Ref<VertexArray>& vertexArray, uint32_t count, uint32_t offset)
	{
		Renderer::Submit([=]() {
			RendererAPI::DrawInstanced(vertexArray, count, offset);
		});
	}

	void Renderer::DrawElementsIndirect(void* indirect)
	{
		Renderer::Submit([=]() {
			RendererAPI::DrawInstancedIndirect(indirect);
		});
	}


	void Renderer::Flush()
	{
		s_Data.CommandQueue.Execute();
	}
	RenderCommandQueue& Renderer::GetRenderCommandQueue()
	{
		return s_Data.CommandQueue;
	}
}