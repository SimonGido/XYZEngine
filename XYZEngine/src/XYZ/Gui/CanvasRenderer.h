#pragma once
#include "XYZ/ECS/Component.h"
#include "XYZ/Renderer/Mesh.h"
#include "XYZ/Renderer/Material.h"
#include "XYZ/Renderer/SubTexture2D.h"

#include <glm/glm.hpp>

namespace XYZ {
	struct CanvasRenderer : public ECS::Type<CanvasRenderer>
	{
		CanvasRenderer(
			Ref<Material>     Material,
			Ref<SubTexture2D> SubTexture,
			glm::vec4		  Color,
			const Mesh&		  mesh,
			int32_t			  sortLayer,
			bool			  isVisible = true
		);

		CanvasRenderer(const CanvasRenderer& other);
		CanvasRenderer(CanvasRenderer&& other) noexcept;


		CanvasRenderer& operator =(const CanvasRenderer& other);
		Ref<Material>     Material;
		Ref<SubTexture2D> SubTexture;
		glm::vec4		  Color;
		Mesh			  Mesh;
		int32_t			  SortLayer = 0;
		float			  TilingFactor = 1.0f;
		bool			  IsVisible = true;
	};
}