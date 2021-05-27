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
			CheckboxChecked,
			CheckboxUnChecked,
			MinimizeButton,
			DownArrow,
			RightArrow,
			LeftArrow,
			Pause,
			NumSubTextures
		};
		enum Colors
		{
			ButtonColor,
			ButtonHighlight,
			CheckboxColor,
			CheckboxHighlight,
			SliderColor,
			SliderHighlight,
			SliderHandleColor,
			SliderHandleHighlight,
			WindowColor,
			WindowHighlight,
			WindowPanelColor,
			WindowPanelHighlight,		
			WindowFrameColor,
			LineColor,
			MenuColor,
			MenuHighlight,
			TabColor,
			TabHighlight,
			InputColor,
			InputHighlight,
			ImageColor,
			ImageHighlight,		
			TextColor,
			TextHighlight,
			DockspaceNodeColor,
			DockspaceNodeHighlight,
			NumColors
		};

		enum ColorTexture
		{
			DefaultTextureIndex,
			WhiteTextureIndex,
			FontTextureIndex,
			NumTextures
		};
		
		Ref<Material>			   Material;
		Ref<Shader>				   LineShader;
		Ref<Texture2D>			   Texture;
		Ref<Texture2D>			   WhiteTexture;
		Ref<Font>				   Font;
		Ref<SubTexture>			   SubTextures[NumSubTextures];
		Ref<SubTexture>			   WhiteSubTexture;

		glm::vec4				   Colors[NumColors];
		glm::vec2				   WindowPadding;
		glm::vec2				   MenuItemSize;
		glm::vec2				   DockspaceNodeSize;
		glm::vec2				   DockspaceNodeOffset;
		float					   TreeNodeOffset;
		float					   TabLabelOffset;
		float					   WindowFrameThickness;
		float					   PanelHeight;
		float					   TabBarHeight;
		float					   MenuBarHeight;
		float					   LabelOffset;		
		float					   ResizeThreshhold;

		friend class InGuiContext;
	};
}