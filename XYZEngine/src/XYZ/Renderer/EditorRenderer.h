#pragma once
#include "Camera.h"
#include "RenderPass.h"
#include "Renderer2D.h"

#include "XYZ/Scene/Components.h"
#include "XYZ/Scene/EditorComponents.h"

namespace XYZ {

	class EditorRenderer : public RefCount
	{
	public:
		void BeginPass(const Ref<RenderPass>& pass, const glm::mat4& viewProjection, const glm::vec3& viewPos);
		void EndPass(Ref<Renderer2D> renderer2D, bool clear = false);

		void SubmitEditorSprite(const Ref<Material>& material, const Ref<SubTexture>& subTexture, const glm::vec4& color, const glm::mat4& transform);
		void SubmitEditorAABB(const glm::vec3& min, const glm::vec3& max, const glm::vec4& color);
		void SubmitEditorLine(const glm::vec3& p0, const glm::vec3& p1, const glm::vec4& color);
		void SubmitEditorCircle(const glm::vec3& pos, float radius, uint32_t sides, const glm::vec4& color);
		void SubmitEditorLineQuad(const glm::vec3& pos, const glm::vec2& size, const glm::vec4& color);
		
	private:
		struct EditorSpriteDrawCommand
		{
			Ref<Material>   Material;
			Ref<SubTexture> SubTexture;
			glm::vec4       Color;
			glm::mat4		Transform;
		};
		struct EditorAABBDrawCommand
		{
			glm::vec3 Min;
			glm::vec3 Max;
			glm::vec4 Color;
		};
		struct EditorLineDrawCommand
		{
			glm::vec3 P0;
			glm::vec3 P1;
			glm::vec4 Color;
		};

		std::vector<EditorSpriteDrawCommand> m_EditorSpriteDrawList;
		std::vector<EditorAABBDrawCommand>   m_EditorAABBDrawList;
		std::vector<EditorLineDrawCommand>   m_EditorLineDrawList;

		Ref<RenderPass> m_Pass;
		glm::mat4		m_ViewProjectionMatrix;
		glm::vec3		m_ViewPosition;

	};
}