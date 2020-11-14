#pragma once

#include "Camera.h"
#include "Material.h"
#include "Mesh.h"
#include "XYZ/Renderer/Font.h"
#include "XYZ/Particle/ParticleEffect.h"


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
		static void BeginScene(const glm::mat4& viewProjectionMatrix, const glm::vec2& viewportSize);

		static uint32_t SetTexture(const Ref<Texture2D>& texture);
		static void SetMaterial(const Ref<Material>& material);
		static void SubmitQuad(const glm::mat4& transform, const glm::vec4& color, float tilingFactor = 1.0f);
		static void SubmitQuad(const glm::mat4& transform, const glm::vec4& texCoord, uint32_t textureID, const glm::vec4& color = glm::vec4(1), float tilingFactor = 1.0f);
		static void SubmitQuad(const glm::vec3& position, const glm::vec2 & size, const glm::vec4& texCoord, uint32_t textureID, const glm::vec4& color = glm::vec4(1), float tilingFactor = 1.0f);
		static void SubmitQuads(const Vertex* vertices, uint32_t countQuads, uint32_t textureID, float tilingFactor = 1.0f);
		static void SubmitQuads(const glm::mat4& transform, const Vertex* vertices, uint32_t countQuads, uint32_t textureID, float tilingFactor = 1.0f);
		static void SubmitLine(const glm::vec3& p0, const glm::vec3& p1, const glm::vec4& color = glm::vec4(1.0f));
		static void SubmitParticles(const glm::mat4& transform, const Ref<ParticleEffect>& particleEffect);
	
		static void SubmitGrid(const glm::mat4& transform, const glm::vec2& scale, float lineWidth);
	

		static void Flush();
		static void FlushLines();
		/**
		* Clean up after rendering
		*/
		static void EndScene();
		
	};
}