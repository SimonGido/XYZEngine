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

	enum RenderQueueType
	{
		Default,
		Overlay,
		NumTypes
	};
	/**
	* @class Renderer
	* @brief represents encapsulation for systems, that takes care of sorting and rendering objects
	*/
	class Renderer
	{
	public:
		/**
		* Initialize RenderCommand and Renderer2D
		*/
		static void Init();
		static void Shutdown();

		static void Clear();
		static void SetClearColor(const glm::vec4& color);
		static void SetViewPort(uint32_t x, uint32_t y, uint32_t width, uint32_t height);
		static void SetLineThickness(float thickness);
		static void SetPointSize(float size);
		static void SetDepthTest(bool val);

		static void DrawArrays(PrimitiveType type, uint32_t count);
		static void DrawIndexed(PrimitiveType type, uint32_t indexCount = 0, uint32_t queueType = Default);
		static void DrawInstanced(const Ref<VertexArray>& vertexArray, uint32_t count, uint32_t offset = 0);
		static void DrawElementsIndirect(void* indirect);
		static void SubmitFullscreenQuad();

		template<typename FuncT>
		static void Submit(FuncT&& func, uint32_t type = Default)
		{		
			auto renderCmd = [](void* ptr) {
				
				auto pFunc = (FuncT*)ptr;
				(*pFunc)();

				pFunc->~FuncT(); // Call destructor
			};
			auto queue = getRenderCommandQueue(type);
			auto storageBuffer = queue.Get().Allocate(renderCmd, sizeof(func));
			new (storageBuffer) FuncT(std::forward<FuncT>(func));
			getStats().CommandsCount++;
		}

		static void BeginRenderPass(const Ref<RenderPass>& renderPass, bool clear);
		static void EndRenderPass();

		static ThreadPool&			GetPool();
		static RendererAPI::API		GetAPI() { return RendererAPI::GetAPI(); }
		static const RendererStats& GetStats();

		static void WaitAndRender();

		// We need this function for now to make sure shaders are created before using them
		static void BlockRenderThread();
	private:
		static ScopedLockReference<RenderCommandQueue> getRenderCommandQueue(uint8_t type);
		static RendererStats&						   getStats();
	};

}