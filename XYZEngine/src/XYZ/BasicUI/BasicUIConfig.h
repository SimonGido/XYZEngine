#pragma once
#include "XYZ/Renderer/Material.h"
#include "XYZ/Renderer/Font.h"
#include "XYZ/Renderer/SubTexture.h"

namespace XYZ {

	class bUIConfig
	{
	public:
		bUIConfig();
		void SetTexture(const Ref<Texture2D>& texture);
		void SetFont(const Ref<Font>& font);
		void SetSubTexture(const Ref<SubTexture>& subTexture, uint8_t index);
		void SetLock(bool lock) { m_Lock = lock; }

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
			NumSubTextures = 32
		};
		enum Colors
		{
			HighlightColor,
			NumColors
		};
		const Ref<SubTexture>& GetSubTexture(uint32_t index) const {return m_SubTextures[index]; }
		const glm::vec4& GetColor(uint32_t index) const {return m_Colors[index]; }
		const Ref<Font>& GetFont() const {return m_Font;}
		bool IsLocked() const { return m_Lock; }
	private:
		Ref<Material>   m_Material;
		Ref<Texture2D>  m_Texture;
		Ref<Font>	    m_Font;
		Ref<SubTexture> m_SubTextures[NumSubTextures];
		glm::vec4		m_Colors[NumColors];
		bool			m_Lock;

		friend class bUI;
		friend class bUIRenderer;
	};

}