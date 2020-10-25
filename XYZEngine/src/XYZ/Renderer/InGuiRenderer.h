#pragma once
#include "Material.h"
#include "Mesh.h"
#include "XYZ/Renderer/Font.h"

namespace XYZ {

	struct UIRenderData
	{
		glm::mat4 ViewProjection;
		glm::vec2 ViewportSize;
	};

	struct InGuiVertex
	{
		glm::vec4 Color;
		glm::vec3 Position;
		glm::vec2 TexCoord;
		float	  TextureID;
		float     TilingFactor = 1.0f;
	};
	struct InGuiLineVertex
	{
		glm::vec3 Position;
		glm::vec4 Color;
	};

	struct TextureRendererIDPair
	{
		uint32_t TextureID;
		uint32_t RendererID;
	};
	struct InGuiMesh
	{
		std::vector<InGuiVertex> Vertices;
	};
	struct InGuiLineMesh
	{
		std::vector<InGuiLineVertex> Vertices;
	};

	class InGuiRenderer
	{
	public:
		/**
		* Initialize Renderer2D instance and register batch system
		*/
		static void Init();

		/**
		* Delete instance of the renderer
		*/
		static void Shutdown();

		/**
		* Initial setup before rendering
		* @param[in] camera		Reference to the camera
		*/
		static void BeginScene(const UIRenderData& data);

		static void SetTexturePairs(const std::vector< TextureRendererIDPair>& texturePairs);
		static void SetMaterial(const Ref<Material>& material);
		static void SubmitUI(const glm::vec2& position, const glm::vec2& size, const glm::vec4& texCoord, uint32_t textureID, const glm::vec4& color = glm::vec4(1));
		static void SubmitUI(uint32_t rendererID, const glm::vec2& position, const glm::vec2& size, const glm::vec4& texCoord, const glm::vec4& color = glm::vec4(1));
		static void SubmitUI(const glm::vec2& position, const Vertex* vertex, size_t count, uint32_t textureID);

		static void SubmitUI(const InGuiMesh& mesh);
		static void SubmitLine(const glm::vec3& p0, const glm::vec3& p1, const glm::vec4& color);
		static void SubmitLineMesh(const InGuiLineMesh& mesh);
		/**
		* Execute the command queue
		*/
		static void Flush();
		static void FlushLines();

		/**
		* Clean up after rendering
		*/
		static void EndScene();

	};
}