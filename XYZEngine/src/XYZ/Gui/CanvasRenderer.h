#pragma once
#include "XYZ/ECS/Component.h"
#include "XYZ/Renderer/SubTexture2D.h"
#include "XYZ/Renderer/Material.h"

#include <glm/glm.hpp>

namespace XYZ {
	struct CanvasRenderer : public Type<CanvasRenderer>
	{
		CanvasRenderer() = default;
		CanvasRenderer(
			Ref<Material> material,
			Ref<SubTexture2D> subTexture,
			const glm::vec4& color,
			uint32_t textureID,
			int32_t sortLayer,
			bool isVisible = true
		);

		CanvasRenderer(const CanvasRenderer& other);
		CanvasRenderer(CanvasRenderer&& other) noexcept;


		CanvasRenderer& operator =(const CanvasRenderer& other);

		Ref<Material> Material;
		Ref<SubTexture2D> SubTexture;
		glm::vec4 Color;

		uint32_t TextureID;
		int32_t SortLayer = 0;
		bool IsVisible = true;

	};
}