#pragma once
#include "XYZ/Renderer/RendererAPI.h"

namespace XYZ {

	class OpenGLRendererAPI : public RendererAPI
	{
	public:
		virtual void Init() override;
		virtual void SetDepth(bool enabled) override;
		virtual void SetScissor(bool enabled) override;
		virtual void SetLineThickness(float thickness) override;
		virtual void SetPointSize(float size) override;
		virtual void SetViewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height) override;
		virtual void SetClearColor(const glm::vec4& color) override;
		virtual void Clear() override;
		virtual void ReadPixels(uint32_t xCoord, uint32_t yCoord, uint32_t width, uint32_t height, uint8_t* data) override;

		virtual void DrawArrays(PrimitiveType type, uint32_t count) override;
		virtual void DrawIndexed(PrimitiveType type, uint32_t indexCount) override;
		virtual void DrawInstanced(PrimitiveType type, uint32_t indexCount, uint32_t instanceCount, uint32_t offset = 0) override;
		virtual void DrawInstancedIndirect(void* indirect) override;
	};

}