#pragma once
#include "Material.h"
#include "Mesh.h"
#include "XYZ/Gui/Font.h"

namespace XYZ {

	struct UIRenderData
	{
		glm::vec2 ViewportSize;
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

		static void SetMaterial(const Ref<Material>& material);
		static void SubmitUI(const glm::vec2& position, const glm::vec2& size, const glm::vec4& texCoord, uint32_t textureID, const glm::vec4& color = glm::vec4(1));
		static void SubmitUI(uint32_t rendererID,const glm::vec2& position, const glm::vec2& size, const glm::vec4& texCoord, const glm::vec4& color = glm::vec4(1));
		static void SubmitUI(const glm::vec2& position,const Vertex* vertex, size_t count, uint32_t textureID);
		
	
		/**
		* Execute the command queue
		*/
		static void Flush();

		/**
		* Clean up after rendering
		*/
		static void EndScene();

	};
}