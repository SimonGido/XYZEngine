#pragma once

#include "Camera.h"
#include "RenderComponent.h"
#include "RenderCommandQueue.h"


namespace XYZ {

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
		static void BeginScene(const Camera& camera);

		/**
		* Submit command to the command queue
		* @param[in] command	Reference to the command
		* @param[in] size		Size of the command
		*/
		static void Submit(CommandI& command, unsigned int size);

		static void SubmitQuad(const glm::vec3& position,const glm::vec2& size, const glm::vec4& texCoord, uint32_t textureID, const glm::vec4& color = glm::vec4(1));
		static void SubmitQuad(const glm::mat4& transform, const glm::vec4& color);
		static void SubmitQuad(const glm::mat4& transform, const glm::vec4& texCoord, uint32_t textureID, const glm::vec4& color = glm::vec4(1));
		static void SubmitQuad(const glm::mat4& transform, const glm::vec3& position, const glm::vec2& size, const glm::vec4& texCoord, uint32_t textureID, const glm::vec4& color = glm::vec4(1));
		
		static void SubmitQuad(const glm::mat4& transform, const Quad& quad);
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