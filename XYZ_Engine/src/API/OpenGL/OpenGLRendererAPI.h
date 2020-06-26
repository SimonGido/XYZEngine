#pragma once
#include "XYZ/Renderer/RendererAPI.h"


namespace XYZ {
	class OpenGLRendererAPI : public RendererAPI
	{
	public:
		virtual void Init() override;
		virtual void SetViewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height) override;

		virtual void SetClearColor(const glm::vec4& color) override;
		virtual void Clear() override;

		virtual void DrawIndexed(const std::shared_ptr<VertexArray>& vertexArray, uint32_t indexCount = 0) override;
		virtual void DrawInstanced(const std::shared_ptr<VertexArray>& vertexArray, uint32_t count,uint32_t offset = 0) override;
		virtual void DrawInstancedIndirect(void* indirect) override;
	};

}