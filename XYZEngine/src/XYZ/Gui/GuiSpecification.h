#pragma once
#include "XYZ/Renderer/Material.h"
#include "XYZ/Renderer/SubTexture2D.h"
#include "XYZ/Gui/Canvas.h"
#include "XYZ/Renderer/Font.h"
#include "XYZ/Gui/Text.h"

#include <glm/glm.hpp>


namespace XYZ {
	struct GuiSpecification
	{
		Ref<Material>  Material;
		Ref<Font>      Font;

		enum Sprites
		{
			BUTTON,
			CHECKBOX_CHECKED,
			CHECKBOX_UNCHECKED,
			SLIDER,
			FONT,
			NUM_SPRITES
		};
		Ref<SubTexture2D> SubTexture[NUM_SPRITES];
	};


	struct CanvasSpecification
	{
		CanvasSpecification(
			CanvasRenderMode mode,
			const glm::vec3& position,
			const glm::vec2& size,
			const glm::vec4& color
		);

		CanvasRenderMode RenderMode;
		glm::vec3		 Position;
		glm::vec2		 Size;
		glm::vec4		 Color;
	};

	struct ButtonSpecification
	{
		ButtonSpecification(
			const std::string& name,
			const glm::vec3& position,
			const glm::vec2& size,
			const glm::vec4& defaultColor,
			const glm::vec4& clickColor,
			const glm::vec4& hooverColor
		);

		std::string Name;
		glm::vec3   Position;
		glm::vec2	Size;

		glm::vec4   DefaultColor;
		glm::vec4   ClickColor;
		glm::vec4   HooverColor;
	};
	struct CheckboxSpecification
	{
		CheckboxSpecification(
			const std::string& name,
			const glm::vec3& position,
			const glm::vec2& size,
			const glm::vec4& defaultColor,
			const glm::vec4& hooverColor
		);

		std::string Name;
		glm::vec3   Position;
		glm::vec2	Size;

		glm::vec4   DefaultColor;
		glm::vec4   HooverColor;
	};
	struct TextSpecification
	{
		TextSpecification(
			TextAlignment alignment,
			const std::string& source,
			const glm::vec3& position,
			const glm::vec2& size,
			const glm::vec4& color
		);

		TextAlignment Alignment;
		std::string Source;
		glm::vec3 Position;
		glm::vec2 Size;
		glm::vec4 Color;
	};

	struct PanelSpecification
	{
		PanelSpecification(
			const glm::vec3& position,
			const glm::vec2& size,
			const glm::vec4& color
		);
		glm::vec3 Position;
		glm::vec2 Size;
		glm::vec4 Color;
	};

}