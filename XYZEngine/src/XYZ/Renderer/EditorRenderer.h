#pragma once
#include "Camera.h"
#include "RenderPass.h"

#include "XYZ/Scene/Components.h"
#include "XYZ/Scene/EditorComponents.h"

namespace XYZ {

	class EditorRenderer
	{
	public:
		static void BeginPass(const Ref<RenderPass>& pass, const glm::mat4& viewProjection, const glm::vec3& viewPos);
		static void EndPass();

		static void SubmitEditorSprite(EditorSpriteRenderer* sprite, TransformComponent* transform);
		static void SubmitEditorAABB(TransformComponent* transform, const glm::vec4& color);
		static void SubmitEditorAABB(const glm::vec3& min, const glm::vec3& max, const glm::vec4& color);
		static void SubmitEditorLine(const glm::vec3& p0, const glm::vec3& p1, const glm::vec4& color);
		static void SubmitEditorCircle(const glm::vec3& pos, float radius, uint32_t sides, const glm::vec4& color);
	};
}