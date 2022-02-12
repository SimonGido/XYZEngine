#pragma once
#include "APIContext.h"
#include "XYZ/Core/Ref/Ref.h"
#include "XYZ/Renderer/VertexArray.h"
#include "XYZ/Renderer/Pipeline.h"
#include "XYZ/Renderer/PipelineCompute.h"
#include "XYZ/Renderer/RenderCommandBuffer.h"
#include "XYZ/Renderer/UniformBufferSet.h"
#include "XYZ/Renderer/StorageBufferSet.h"
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
		std::string Device;
		std::string Version;

		int   MaxSamples = 0;
		float MaxAnisotropy = 0.0f;
		int   MaxTextureUnits = 0;
	};


	class RendererAPI
	{
	public:
		enum class Type
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
		
		virtual void RenderGeometry(Ref<RenderCommandBuffer> renderCommandBuffer, Ref<Pipeline> pipeline, Ref<Material> material, Ref<VertexBuffer> vertexBuffer, Ref<IndexBuffer> indexBuffer, const glm::mat4& transform, uint32_t indexCount = 0, uint32_t vertexOffsetSize = 0) {};
		virtual void RenderGeometry(Ref<RenderCommandBuffer> renderCommandBuffer, Ref<Pipeline> pipeline, Ref<Material> material, Ref<VertexBuffer> vertexBuffer, Ref<IndexBuffer> indexBuffer, uint32_t indexCount = 0) {};
		virtual void RenderMesh(Ref<RenderCommandBuffer> renderCommandBuffer, Ref<Pipeline> pipeline, Ref<Material> material, Ref<VertexBuffer> vertexBuffer, Ref<IndexBuffer> indexBuffer, Ref<VertexBuffer> transformBuffer, uint32_t transformOffset, uint32_t instanceCount) {};
		virtual void RenderMesh(
			Ref<RenderCommandBuffer> renderCommandBuffer, Ref<Pipeline> pipeline, Ref<Material> material,
			Ref<VertexBuffer> vertexBuffer, Ref<IndexBuffer> indexBuffer, const glm::mat4& transform,
			Ref<VertexBuffer> instanceBuffer, uint32_t instanceOffset, uint32_t instanceCount
		) {};
		virtual void RenderMesh(
			Ref<RenderCommandBuffer> renderCommandBuffer, Ref<Pipeline> pipeline, Ref<Material> material,
			Ref<VertexBuffer> vertexBuffer, Ref<IndexBuffer> indexBuffer,
			Ref<VertexBuffer> transformBuffer, uint32_t transformOffset, uint32_t transformInstanceCount,
			Ref<VertexBuffer> instanceBuffer, uint32_t instanceOffset, uint32_t instanceCount
		) {};

		virtual void BindPipeline(Ref<RenderCommandBuffer> renderCommandBuffer, Ref<Pipeline> pipeline, Ref<UniformBufferSet> uniformBufferSet, Ref<StorageBufferSet> storageBufferSet, Ref<Material> material) {};
		virtual void BeginPipelineCompute(Ref<RenderCommandBuffer> renderCommandBuffer, Ref<PipelineCompute> pipeline, Ref<UniformBufferSet> uniformBufferSet, Ref<StorageBufferSet> storageBufferSet, Ref<Material> material) {};
		virtual void DispatchCompute(Ref<PipelineCompute> pipeline, Ref<Material> material, uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ) {};
		virtual void EndPipelineCompute(Ref<PipelineCompute> pipeline) {};
		virtual void UpdateDescriptors(Ref<PipelineCompute> pipeline, Ref<Material> material, Ref<UniformBufferSet> uniformBufferSet, Ref<StorageBufferSet> storageBufferSet) {};
		virtual void ClearImage(Ref<RenderCommandBuffer> renderCommandBuffer, Ref<Image2D> image) {};

		static const RenderAPICapabilities& GetCapabilities();
		

		static Type GetType() { return s_API; }
		
	protected:
		static RenderAPICapabilities& getCapabilities();


	private:
		static Type s_API;

	};
}