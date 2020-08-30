#include <memory>
#include <glm/glm.hpp>

#include "XYZ/Core/Ref.h"
#include "RendererAPI.h"


namespace XYZ {

	
	/**
	* @class RenderCommand
	* @brief holds RendererAPI, provide static functions to call RendererAPI function
	*/
	class RenderCommand
	{
	public:
		/**
		* Initialize RendererAPI
		*/
		inline static void Init()
		{
			s_RendererAPI->Init();
		}

		/**
		* Set the viewport 
		* @param[in] x		 Position x 
		* @param[in] y		 Position y
		* @param[in] width   Width of the viewport
		* @param[in] height  Height of the viewport
		*/
		inline static void SetViewPort(uint32_t x, uint32_t y, uint32_t width, uint32_t height)
		{
			s_RendererAPI->SetViewport(x, y, width, height);
		}

		/**
		* Set all the pixels to the specified color
		* @param[in] color Color of the pixels RGBA format
		*/
		inline static void SetClearColor(const glm::vec4& color)
		{
			s_RendererAPI->SetClearColor(color);
		}

		/**
		* Clear depth and color buffer
		*/
		inline static void Clear()
		{
			s_RendererAPI->Clear();
		}

		/**
		* Draw the data in the vertex array indexed, vertex array must have vertex buffer and index buffer
		* @param[in] vertexArray
		* @param[in] indexCount		Count of the indices, if zero function takes count of the indices from the vertex array
		*/
		inline static void DrawIndexed(PrimitiveType type, uint32_t indexCount = 0)
		{
			s_RendererAPI->DrawIndexed(type, indexCount);
		}

		/**
		* Draw the specified number of the instances of the vertices in vertex array
		* @param[in] vertexArray
		* @param[in] count		Count of the instances
		*/
		inline static void DrawInstanced(const Ref<VertexArray>& vertexArray, uint32_t count,uint32_t offset =0)
		{
			s_RendererAPI->DrawInstanced(vertexArray, count,offset);
		}


		inline static void DrawElementsIndirect(void* indirect)
		{
			s_RendererAPI->DrawInstancedIndirect(indirect);
		}
	private:
		static std::unique_ptr<RendererAPI> s_RendererAPI;
	};

}