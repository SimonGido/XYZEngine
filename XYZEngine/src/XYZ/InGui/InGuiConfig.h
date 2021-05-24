#pragma once
#include "XYZ/Renderer/Texture.h"
#include "XYZ/Renderer/Font.h"
#include "XYZ/Renderer/SubTexture.h"
#include "XYZ/Renderer/Material.h"
#include "XYZ/Renderer/Shader.h"

namespace XYZ {
	class InGuiConfig
	{
	public:
		InGuiConfig();

		enum SubTextures
		{
			Button = 0,
			CheckboxChecked,
			CheckboxUnChecked,
			Slider,
			SliderHandle,
			Window,
			MinimizeButton,
			CloseButton,
			DownArrow,
			RightArrow,
			LeftArrow,
			Pause,
			White,
			NumSubTextures = 32
		};
		enum Colors
		{
			ButtonDefault,
			ButtonHoover,
			CheckboxDefault,
			CheckboxHoover,
			SliderDefault,
			SliderHoover,
			WindowDefault,
			WindowHoover,
			WindowFrameColor,
			LineColor,
			MenuDefault,
			MenuHoover,
			TabDefault,
			TabHoover,
			InputDefault,
			InputHoover,
			ImageDefault,
			ImageHoover,		
			TextDefault,
			TextHighlight,
			NumColors
		};

		static constexpr uint32_t sc_DefaultTexture = 0;
		static constexpr uint32_t sc_FontTexture = 1;
		

		Ref<Material>			   Material;
		Ref<Shader>				   LineShader;
		Ref<Texture2D>			   Texture;
		Ref<Font>				   Font;
		Ref<SubTexture>			   SubTextures[NumSubTextures];
		glm::vec4				   Colors[NumColors];
		glm::vec2				   WindowPadding;
		glm::vec2				   MenuItemSize;
		float					   TreeNodeOffset;
		float					   TabLabelOffset;
		float					   WindowFrameThickness;
		float					   PanelHeight;
		float					   TabBarHeight;
		float					   MenuBarHeight;
		float					   LabelOffset;		

		friend class InGuiContext;
	};
}