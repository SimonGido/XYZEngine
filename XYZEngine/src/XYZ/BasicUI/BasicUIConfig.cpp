#include "stdafx.h"
#include "BasicUIConfig.h"


namespace XYZ {
	bUIConfig::bUIConfig()
		:
		m_Lock(false)
	{
		m_Material = Ref<XYZ::Material>::Create(Shader::Create("Assets/Shaders/InGuiShader.glsl"));
		m_Material->Set("u_Color", glm::vec4(2.0f));

		m_Colors[HighlightColor] = { 1.0f, 1.9f, 2.1, 1.0f };
	}
	void bUIConfig::SetTexture(const Ref<Texture2D>& texture)
	{
		m_Material->Set("u_Texture", texture, 0);
		m_Texture = texture;
	}
	void bUIConfig::SetFont(const Ref<Font>& font)
	{
		m_Material->Set("u_Texture", font->GetTexture(), 1);
		m_Font = font;
	}
	void bUIConfig::SetSubTexture(const Ref<SubTexture>& subTexture, uint8_t index)
	{
		XYZ_ASSERT(index < NumSubTextures, "");
		XYZ_ASSERT(subTexture->GetTexture().Raw() == m_Texture.Raw(), "");
		m_SubTextures[index] = subTexture;
	}
}