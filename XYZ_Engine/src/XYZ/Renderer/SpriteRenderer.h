#pragma once
#include "XYZ/ECS/Component.h"

#include "SubTexture2D.h"
#include "SortLayer.h"
#include "Material.h"



namespace XYZ {

	struct SpriteRenderer : public Type<SpriteRenderer>
	{
		SpriteRenderer(Ref<Material> material,
					   Ref<SubTexture2D> subTexture,
					   uint32_t textureID,
					   SortLayerID layer,
					   bool isVisible = true)
			:
			Material(material),
			SubTexture(subTexture),
			TextureID(textureID),
			Layer(layer),
			IsVisible(isVisible)
		{
		}
		SpriteRenderer() = default;

		Ref<Material> Material;
		Ref<SubTexture2D> SubTexture;

		uint32_t TextureID;
		SortLayerID Layer = 0;
		bool IsVisible = true;
	};
}