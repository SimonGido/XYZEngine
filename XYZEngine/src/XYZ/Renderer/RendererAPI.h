#pragma once
#include "APIContext.h"
#include "XYZ/Core/Ref.h"
#include "XYZ/Renderer/VertexArray.h"

namespace XYZ {

	enum class PrimitiveType
	{
		None,
		Triangles,
		Lines
	};

	/**
	* @interface RendererAPI
	* pure virtual (interface) class.
	*/
	class RendererAPI
	{
	public:
		enum class API
		{
			None = 0, OpenGL = 1
		};
	public:
		virtual void Init() = 0;
		virtual void SetViewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height) = 0;
		virtual void SetClearColor(const glm::vec4& color) = 0;
		virtual void Clear() = 0;

		virtual void DrawIndexed(PrimitiveType type, uint32_t indexCount) = 0;
		virtual void DrawInstanced(const Ref<VertexArray>& vertexArray, uint32_t count,uint32_t offset=0) = 0;
		virtual void DrawInstancedIndirect(void* indirect) = 0;


		inline static API GetAPI() { return s_API; }
		/**
		* @return unique_ptr to RendererAPI 
		*/
		static std::unique_ptr<RendererAPI> Create();
	private:
		static API s_API;
	};
}