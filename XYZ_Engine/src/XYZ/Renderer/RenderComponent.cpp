#include "stdafx.h"
#include "RenderComponent.h"


namespace XYZ {
	Quad::Quad(const glm::vec2& position, const glm::vec2& size, const glm::vec4& color, int32_t textureID)
	{
		Vertices[0].Position = { position.x - (size.x / 2), position.y - (size.y / 2), 0.0f, 1.0f };
		Vertices[1].Position = { position.x + (size.x / 2), position.y - (size.y / 2), 0.0f, 1.0f };
		Vertices[2].Position = { position.x + (size.x / 2), position.y + (size.y / 2), 0.0f, 1.0f };
		Vertices[3].Position = { position.x - (size.x / 2), position.y + (size.y / 2), 0.0f, 1.0f };

		Vertices[0].Color = color;
		Vertices[1].Color = color;
		Vertices[2].Color = color;
		Vertices[3].Color = color;

		Vertices[0].TextureID = textureID;
		Vertices[1].TextureID = textureID;
		Vertices[2].TextureID = textureID;
		Vertices[3].TextureID = textureID;
	}
	Quad::Quad(const glm::vec2& position, const glm::vec2& size, const glm::vec4& color, int32_t textureID, bool centered)
	{
		Vertices[0].Position = { position.x , position.y , 0.0f, 1.0f };
		Vertices[1].Position = { position.x + size.x, position.y, 0.0f, 1.0f };
		Vertices[2].Position = { position.x + size.x, position.y + size.y, 0.0f, 1.0f };
		Vertices[3].Position = { position.x , position.y + size.y, 0.0f, 1.0f };

		Vertices[0].Color = color;
		Vertices[1].Color = color;
		Vertices[2].Color = color;
		Vertices[3].Color = color;

		Vertices[0].TextureID = textureID;
		Vertices[1].TextureID = textureID;
		Vertices[2].TextureID = textureID;
		Vertices[3].TextureID = textureID;
	}
	Quad::Quad(const Vertex* vertices)
	{
		for (size_t i = 0; i < 4; ++i)
			Vertices[i] = vertices[i];
	}
	RenderComponent::RenderComponent(Ref<Material> material, SortLayerID layer, bool visible)
		:
		MaterialIns(material),
		Layer(layer),
		IsVisible(visible)
	{
	}
}