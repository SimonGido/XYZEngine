#pragma once
#include "SubTexture2D.h"
#include "SortingLayer.h"
#include "Material.h"

#include <glm/glm.hpp>

namespace XYZ {

	struct Vertex
	{
		glm::vec4 Color;
		glm::vec4 Position;
		glm::vec2 TexCoord;
		int32_t	  TextureID;
	};

	struct Quad
	{
		Quad(int32_t textureID, const glm::vec4& color, const glm::vec2& position = glm::vec2(0), const glm::vec2& size = glm::vec2(1));
		Quad(const glm::vec2& position, const glm::vec2& size, const glm::vec4& color, int32_t textureID, bool centered);
		Quad(const Vertex* vertices);

		Vertex Vertices[4];
	};

	class RenderComponent
	{
	public:
		RenderComponent(
			const Ref<Material>& material,
			SortLayerID layer,
			bool visible
		);

		virtual ~RenderComponent() {};
		// Sort data
		Ref<Material> MaterialIns;
		SortLayerID Layer;
		bool IsVisible;


		// Get render data
		virtual const Quad* GetRenderData() const = 0;
		virtual size_t GetCountQuads() const { return 1; }
	};

}