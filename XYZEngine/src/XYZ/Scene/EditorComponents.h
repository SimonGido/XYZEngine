#pragma once
#include "XYZ/ECS/ECSManager.h"
#include "XYZ/ECS/Component.h"
#include "XYZ/Core/GUID.h"
#include "XYZ/Renderer/SubTexture.h"
#include "XYZ/Renderer/Material.h"
#include "XYZ/Editor/EditorCamera.h"

namespace XYZ {
	struct EditorSpriteRenderer
	{
		EditorSpriteRenderer() = default;
		EditorSpriteRenderer(
			const Ref<Material>&material,
			const Ref<SubTexture>&subTexture,
			const glm::vec4 & color,
			bool isVisible = true
		);

		EditorSpriteRenderer(const EditorSpriteRenderer & other);
		EditorSpriteRenderer(EditorSpriteRenderer && other) noexcept;


		EditorSpriteRenderer& operator =(const EditorSpriteRenderer & other);

		Ref<Material>	Material;
		Ref<SubTexture> SubTexture;
		glm::vec4	    Color = glm::vec4(1.0f); 
	};
}