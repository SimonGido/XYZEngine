#pragma once
#include "XYZ/ECS/Component.h"
#include "XYZ/Renderer/Mesh.h"
#include "XYZ/Renderer/Material.h"

#include <glm/glm.hpp>

namespace XYZ {
	struct CanvasRenderer : public Type<CanvasRenderer>
	{
		CanvasRenderer(
			Ref<Material> material,
			const Mesh& mesh,
			uint32_t textureID,
			int32_t sortLayer,
			bool isVisible = true
		);

		CanvasRenderer(const CanvasRenderer& other);
		CanvasRenderer(CanvasRenderer&& other) noexcept;


		CanvasRenderer& operator =(const CanvasRenderer& other);

		Ref<Material> Material;
		Mesh	      Mesh;
		uint32_t      TextureID;
		int32_t       SortLayer = 0;
		float         TilingFactor = 1.0f;
		bool          IsVisible = true;
	};
}