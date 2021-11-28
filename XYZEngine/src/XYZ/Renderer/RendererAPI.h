#pragma once
#include "APIContext.h"
#include "XYZ/Core/Ref.h"
#include "XYZ/Renderer/VertexArray.h"
#include "XYZ/Renderer/Pipeline.h"
#include "XYZ/Renderer/RenderCommandBuffer.h"
#include "XYZ/Renderer/UniformBufferSet.h"
#include "XYZ/Renderer/Material.h"

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
		virtual void Shutdown() {}
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
		virtual void BeginRenderPass(Ref<RenderCommandBuffer> renderCommandBuffer, Ref<RenderPass> renderPass, bool explicitClear = false) {};
		virtual void EndRenderPass(Ref<RenderCommandBuffer> renderCommandBuffer) {};
		virtual void RenderGeometry(Ref<RenderCommandBuffer> renderCommandBuffer, Ref<Pipeline> pipeline, Ref<Material> material, Ref<VertexBuffer> vertexBuffer, Ref<IndexBuffer> indexBuffer, const glm::mat4& transform, uint32_t indexCount = 0) {};
		virtual void RenderGeometry(Ref<RenderCommandBuffer> renderCommandBuffer, Ref<Pipeline> pipeline, Ref<Material> material, Ref<VertexBuffer> vertexBuffer, Ref<IndexBuffer> indexBuffer, uint32_t indexCount = 0) {};
		virtual void BindPipeline(Ref<RenderCommandBuffer> renderCommandBuffer, Ref<Pipeline> pipeline, Ref<UniformBufferSet> uniformBufferSet, Ref<Material> material) {};

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