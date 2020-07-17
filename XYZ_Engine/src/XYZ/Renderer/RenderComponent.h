#pragma once
#include "XYZ/ECS/Component.h"

#include "SubTexture2D.h"
#include "SortLayer.h"
#include "Material.h"
#include "Mesh.h"

#include <glm/glm.hpp>

namespace XYZ {


	struct RenderComponent2D : public Type<RenderComponent2D>
	{
		RenderComponent2D(Ref<Material> material,
						  Ref<Mesh> mesh,
						  SortLayerID layer,
						  bool isVisible = true)
			:
			Material(material),
			Mesh(mesh),
			Layer(layer),
			IsVisible(isVisible)
		{}
		RenderComponent2D() = default;

		Ref<Material> Material;
		Ref<Mesh> Mesh;

		SortLayerID Layer = 0;
		bool IsVisible = true;
	};
}