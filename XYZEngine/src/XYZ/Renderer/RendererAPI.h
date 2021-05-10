#pragma once
#include "APIContext.h"
#include "XYZ/Core/Ref.h"
#include "XYZ/Renderer/VertexArray.h"

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

		int MaxSamples = 0;
		float MaxAnisotropy = 0.0f;
		int MaxTextureUnits = 0;
	};


	class RendererAPI
	{
	public:
		enum class API
		{
			None = 0, OpenGL = 1
		};
	public:
		static void Init();
		static void SetDepth(bool enabled);
		static void SetScissor(bool enabled);
		static void SetLineThickness(float thickness);
		static void SetPointSize(float size);
		static void SetViewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height);
		static void SetClearColor(const glm::vec4& color);
		static void Clear();
		static void ReadPixels(uint32_t xCoord, uint32_t yCoord, uint32_t width, uint32_t height, uint8_t* data);

		static void DrawArrays(PrimitiveType type, uint32_t count);
		static void DrawIndexed(PrimitiveType type, uint32_t indexCount);
		static void DrawInstanced(const Ref<VertexArray>& vertexArray, uint32_t count, uint32_t offset = 0);
		static void DrawInstancedIndirect(void* indirect);
	

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