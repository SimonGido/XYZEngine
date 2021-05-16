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
		void SetTexture(const Ref<Texture2D>& texture);
		void SetFont(const Ref<Font>& font);
		void SetSubTexture(const Ref<SubTexture>& subTexture, uint8_t index);

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
			DefaultColor,
			HighlightColor,
			TextHighlightColor,
			NumColors
		};

		const Ref<SubTexture>& GetSubTexture(uint32_t index) const { return m_SubTextures[index]; }
		const glm::vec4& GetColor(uint32_t index) const { return m_Colors[index]; }
		const Ref<Font>& GetFont() const { return m_Font; }
	
		static constexpr uint32_t sc_DefaultTexture = 0;
		static constexpr uint32_t sc_FontTexture = 1;
	private:
		Ref<Material>			 m_Material;
		Ref<Shader>			     m_LineShader;
		Ref<Texture2D>			 m_Texture;
		Ref<Font>				 m_Font;
		Ref<SubTexture>			 m_SubTextures[NumSubTextures];
		glm::vec4				 m_Colors[NumColors];

		friend class InGuiContext;
	};
}