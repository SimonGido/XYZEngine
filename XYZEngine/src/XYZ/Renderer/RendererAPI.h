#pragma once
#include "APIContext.h"
#include "XYZ/Core/Ref.h"
#include "XYZ/Renderer/VertexArray.h"
#include "XYZ/Renderer/Pipeline.h"
#include "XYZ/Renderer/RenderCommandBuffer.h"

namespace XYZ {

	enum class PrimitiveType
	{
		None,
		Triangles,
		Lines,
		Points
	};

	struct RenderAPICapabilities
	{
		std::string Vendor;
		std::string Renderer;
		std::string Version;

		int   MaxSamples = 0;
		float MaxAnisotropy = 0.0f;
		int   MaxTextureUnits = 0;
	};


	class RendererAPI
	{
	public:
		enum class API
		{
			None = 0, OpenGL = 1, Vulkan = 2
		};
	public:
		virtual ~RendererAPI() = default;
		virtual void Init() = 0;
		virtual void SetDepth(bool enabled) {};
		virtual void SetScissor(bool enabled) {};
		virtual void SetLineThickness(float thickness) {};
		virtual void SetPointSize(float size) {};
		virtual void SetViewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height) {};
		virtual void SetClearColor(const glm::vec4& color) {};
		virtual void Clear() {};
		virtual void ReadPixels(uint32_t xCoord, uint32_t yCoord, uint32_t width, uint32_t height, uint8_t* data) {};

		virtual void DrawArrays(PrimitiveType type, uint32_t count) {};
		virtual void DrawIndexed(PrimitiveType type, uint32_t indexCount) {};
		virtual void DrawInstanced(PrimitiveType type, uint32_t indexCount, uint32_t instanceCount, uint32_t offset = 0) {};
		virtual void DrawInstancedIndirect(void* indirect) {};
	
		// New API
		virtual void BeginFrame() {}
		virtual void EndFrame() {}
	
		
		virtual void TestDraw(const Ref<RenderPass>& renderPass, const Ref<RenderCommandBuffer>& commandBuffer, const Ref<Pipeline>& pipeline, const Ref<VertexBuffer>& vbo, const Ref<IndexBuffer>& ibo) {};

		static RenderAPICapabilities& GetCapabilities()
		{
			static RenderAPICapabilities caps;
			return caps;
		}
		
		static API GetAPI() { return s_API; }
		/**
		* @return unique_ptr to RendererAPI 
		*/
	private:
		static API s_API;
	};
}