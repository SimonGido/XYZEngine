#pragma once
#include <memory>

#include "XYZ/Core/ThreadPool.h"
#include "XYZ/Utils/DataStructures/ThreadPass.h"


#include "Shader.h"
#include "Camera.h"
#include "VertexArray.h"
#include "RendererAPI.h"
#include "RenderCommandQueue.h"
#include "RenderPass.h"
#include "Pipeline.h"
#include "Material.h"

namespace XYZ {

	struct RendererStats
	{
		RendererStats();
		void Reset();

		uint32_t DrawArraysCount;
		uint32_t DrawIndexedCount;
		uint32_t DrawInstancedCount;
		uint32_t DrawFullscreenCount;
		uint32_t DrawIndirectCount;

		uint32_t CommandsCount;
	};

	struct RendererConfiguration
	{
		uint32_t FramesInFlight = 3;
	};

	class Renderer
	{
	public:
		static void Init(const RendererConfiguration& config = RendererConfiguration());
		static void InitResources();
		static void Shutdown();

		// Old API
		static void Clear();
		static void SetClearColor(const glm::vec4& color);
		static void SetViewPort(uint32_t x, uint32_t y, uint32_t width, uint32_t height);
		static void SetLineThickness(float thickness);
		static void SetPointSize(float size);
		static void SetDepthTest(bool val);

		static void DrawArrays(PrimitiveType type, uint32_t count);
		static void DrawIndexed(PrimitiveType type, uint32_t indexCount = 0);
		static void DrawInstanced(PrimitiveType type, uint32_t indexCount, uint32_t instanceCount, uint32_t offset = 0);
		static void DrawElementsIndirect(void* indirect);
		static void SubmitFullscreenQuad();
		/////////////
		


		// New API //
		static void BeginFrame();
		static void EndFrame();

		static void BeginRenderPass(Ref<RenderCommandBuffer> renderCommandBuffer, const Ref<RenderPass>& renderPass, bool clear);
		static void EndRenderPass(Ref<RenderCommandBuffer> renderCommandBuffer);
		static void RenderGeometry(Ref<RenderCommandBuffer> renderCommandBuffer, Ref<Pipeline> pipeline, Ref<UniformBufferSet> uniformBufferSet, Ref<Material> material, Ref<VertexBuffer> vertexBuffer, Ref<IndexBuffer> indexBuffer, uint32_t indexCount = 0);
		

		static void RegisterShaderDependency(const Ref<Shader>& shader, const Ref<Pipeline>& pipeline);
		static void RegisterShaderDependency(const Ref<Shader>& shader, const Ref<Material>& material);
		static void RemoveShaderDependency(size_t hash);
		static void OnShaderReload(size_t hash);
		//////////////

		static Ref<ShaderLibrary>			GetShaderLibrary();
		static Ref<APIContext>				GetAPIContext();
		static const RenderAPICapabilities& GetCapabilities();
		static const RendererConfiguration& GetConfiguration();

		template<typename FuncT>
		static void Submit(FuncT&& func);

		template<typename FuncT>
		static void SubmitAndWait(FuncT&& func);

		template<typename FuncT>
		static void SubmitResource(FuncT&& func);

		static void BlockRenderThread();
		static void WaitAndRenderAll();
		static void WaitAndRender();
		static void Render();
		static void HandleResources();


		static ThreadPool&			GetPool();
		static RendererAPI::API		GetAPI() { return RendererAPI::GetAPI(); }
		static RendererAPI*			GetRendererAPI();
		static const RendererStats& GetStats();
		static RenderCommandQueue&	GetRenderResourceQueue(uint32_t index);

	private:
		static RenderCommandQueue& getRenderCommandQueue();
		static RendererStats&	   getStats();
	};

	template <typename FuncT>
	void Renderer::Submit(FuncT&& func)
	{
		auto renderCmd = [](void* ptr) {

			auto pFunc = static_cast<FuncT*>(ptr);
			(*pFunc)();
			pFunc->~FuncT(); // Call destructor
		};
		auto& queue = getRenderCommandQueue();
		auto storageBuffer = queue.Allocate(renderCmd, sizeof(func));
		new (storageBuffer) FuncT(std::forward<FuncT>(func));
		getStats().CommandsCount++;
	}

	template <typename FuncT>
	void Renderer::SubmitAndWait(FuncT&& func)
	{
		Submit(std::forward<FuncT>(func));
		WaitAndRender();
		BlockRenderThread();
	}

	template <typename FuncT>
	void Renderer::SubmitResource(FuncT&& func)
	{
		auto renderCmd = [](void* ptr) {
			auto pFunc = static_cast<FuncT*>(ptr);
			(*pFunc)();
			pFunc->~FuncT();
		};

		Submit([renderCmd, func]()
		{
			const uint32_t index = Renderer::GetAPIContext()->GetCurrentFrame();
			auto storageBuffer = GetRenderResourceQueue(index).Allocate(renderCmd, sizeof(func));
			new (storageBuffer) FuncT(std::forward<FuncT>((FuncT&&)func));
		});
	}

	namespace Utils {

		inline void DumpGPUInfo()
		{
			auto& caps = Renderer::GetCapabilities();
			XYZ_TRACE("GPU Info:");
			XYZ_TRACE("  Vendor: {0}", caps.Vendor);
			XYZ_TRACE("  Device: {0}", caps.Renderer);
			XYZ_TRACE("  Version: {0}", caps.Version);
		}
	}

}