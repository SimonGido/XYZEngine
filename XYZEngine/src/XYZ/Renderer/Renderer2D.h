#pragma once

#include "Camera.h"
#include "RenderCommandQueue.h"
#include "Material.h"
#include "Mesh.h"


namespace XYZ {

	struct SceneRenderData
	{
		glm::mat4 ViewProjectionMatrix;
		glm::vec2 ViewportSize;
	};

	class Renderer2D
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
		static void BeginScene(const SceneRenderData& data);

		static void SetMaterial(const Ref<Material>& material);
	

		static void SubmitQuad(const glm::mat4& transform, const glm::vec4& color);
		static void SubmitQuad(const glm::mat4& transform, const glm::vec4& texCoord, uint32_t textureID, const glm::vec4& color = glm::vec4(1));
		static void SubmitQuad(const glm::mat4& transform, const glm::vec3& position, const glm::vec2& size, const glm::vec4& texCoord, uint32_t textureID, const glm::vec4& color = glm::vec4(1));
		static void SubmitLine(const glm::vec3& p0, const glm::vec3& p1, const glm::vec4& color = glm::vec4(1.0f));
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