#pragma once
#include "XYZ/ECS/Component.h"
#include "SubTexture2D.h"
#include "SortingLayer.h"
#include "Material.h"

#include <glm/glm.hpp>

namespace XYZ {
	/**
	* @struct Renderable2D
	* @brief represents 2D renderable object.
	*/
	struct Renderable2D : public Type<Renderable2D>
	{
		/**
		* Construct a 2D renderable, from given parameters
		* @param[in] material	Material of the renderable
		* @param[in] color		Color of the renderable
		* @param[in] subTexture	Sub texture
		* @param[in] position   Position of the renderable
		* @param[in] size		Size of the renderable
		* @param[in] rotation   Rotation of the renderable
		* @param[in] visible	Specify if the renderable is visible and should be rendered
		* @param[in] textureID  Specify the ID of the texture
		*/
		Renderable2D(
			std::shared_ptr<Material> material,
			std::shared_ptr<SubTexture2D> subTexture,
			const glm::vec4& color,
			bool visible,
			int textureID
			)
		:
			Material(material),
			SubTexture(subTexture),
			Color(color),
			Visible(visible),
			TextureID(textureID),
			SortLayerID(SortingLayer::Get().GetOrderValueByName("default"))
		{}

		~Renderable2D()
		{
		}

		std::shared_ptr<Material> Material;
		std::shared_ptr<SubTexture2D> SubTexture;
		glm::vec4 Color = glm::vec4(0);
		bool Visible = false;
		int TextureID = 0;
		SortingLayerID SortLayerID = SortingLayer::Get().GetOrderValueByName("default");
	};
}