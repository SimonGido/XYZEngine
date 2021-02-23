#pragma once
#include <memory>

#include "Shader.h"
#include "Camera.h"
#include "VertexArray.h"
#include "RendererAPI.h"
#include "RenderCommandQueue.h"
#include "RenderPass.h"

namespace XYZ {

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
		static void SetPointSize(float size);
		static void SetDepthTest(bool val);
		static void SetScissorTest(bool val);
		static void Scissor(uint32_t x, uint32_t y, uint32_t width, uint32_t height);
		static void ScissorArray(uint32_t count, ScissorSpecs* scissors);

		static void DrawArrays(PrimitiveType type, uint32_t count);
		static void DrawIndexed(PrimitiveType type, uint32_t indexCount = 0);
		static void DrawInstanced(const Ref<VertexArray>& vertexArray, uint32_t count, uint32_t offset = 0);
		static void DrawElementsIndirect(void* indirect);
		static void SubmitFullsceenQuad();

		template<typename FuncT>
		static void Submit(FuncT&& func)
		{
			auto renderCmd = [](void* ptr) {
				auto pFunc = (FuncT*)ptr;
				(*pFunc)();

				pFunc->~FuncT(); // Call destructor
			};
			auto storageBuffer = GetRenderCommandQueue().Allocate(renderCmd, sizeof(func));
			new (storageBuffer) FuncT(std::forward<FuncT>(func));
		}

		static void BeginRenderPass(const Ref<RenderPass>& renderPass, bool clear);
		static void EndRenderPass();

		/**
		* @return RendererAPI
		*/
		static RendererAPI::API GetAPI() { return RendererAPI::GetAPI(); }

		/**
		* Execute the command queue
		*/
		static void WaitAndRender();

	private:
		static RenderCommandQueue& GetRenderCommandQueue();
	};

}