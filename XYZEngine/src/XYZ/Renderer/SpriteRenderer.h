#pragma once
#include "XYZ/ECS/Component.h"

#include "SubTexture2D.h"
#include "Material.h"



namespace XYZ {

	struct SpriteRenderer : public Type<SpriteRenderer>
	{
		SpriteRenderer() = default;

		SpriteRenderer(
			Ref<Material> material,
			Ref<SubTexture2D> subTexture,
			const glm::vec4& color,
			uint32_t textureID,
			int32_t sortLayer,
			bool isVisible = true
		);
			
		SpriteRenderer(const SpriteRenderer& other);
		SpriteRenderer(SpriteRenderer&& other) noexcept;
			

		SpriteRenderer& operator =(const SpriteRenderer& other);
		

		Ref<Material> Material;
		Ref<SubTexture2D> SubTexture;
		glm::vec4 Color;

		uint32_t TextureID;
		int32_t SortLayer = 0;
		bool IsVisible = true;
	};
}