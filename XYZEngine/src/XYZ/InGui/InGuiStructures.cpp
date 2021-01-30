#include "stdafx.h"
#include "InGuiStructures.h"


namespace XYZ {

	static uint32_t s_NextID = 0;
	static std::queue<uint32_t> s_FreeIDs;

	InGuiRenderData::InGuiRenderData()
	{
		Ref<Shader> shader = Shader::Create("Assets/Shaders/DefaultShader.glsl");
		Texture = Texture2D::Create({ TextureWrap::Clamp, TextureParam::Linear, TextureParam::Nearest }, "Assets/Textures/Gui/TexturePack_Dark.png");
		Ref<Texture2D> colorPickerTexture = Texture2D::Create({ TextureWrap::Clamp, TextureParam::Nearest, TextureParam::Nearest }, "Assets/Textures/Gui/ColorPicker.png");

		Font = Ref<XYZ::Font>::Create(14, "Assets/Fonts/arial.ttf");
		Material = Ref<XYZ::Material>::Create(shader);
		Material->Set("u_Texture", Texture, TextureID);
		Material->Set("u_Texture", Font->GetTexture(), FontTextureID);
		Material->Set("u_Texture", colorPickerTexture, ColorPickerTextureID);
		Material->Set("u_Color", glm::vec4(1.0f));


		float divisor = 8.0f;
		SubTexture[BUTTON] = Ref<XYZ::SubTexture>::Create(Texture, glm::vec2(0, 0), glm::vec2(Texture->GetWidth() / divisor, Texture->GetHeight() / divisor));
		SubTexture[CHECKBOX_CHECKED] = Ref<XYZ::SubTexture>::Create(Texture, glm::vec2(1, 1), glm::vec2(Texture->GetWidth() / divisor, Texture->GetHeight() / divisor));
		SubTexture[CHECKBOX_UNCHECKED] = Ref<XYZ::SubTexture>::Create(Texture, glm::vec2(0, 1), glm::vec2(Texture->GetWidth() / divisor, Texture->GetHeight() / divisor));
		SubTexture[SLIDER] = Ref<XYZ::SubTexture>::Create(Texture, glm::vec2(0, 0), glm::vec2(Texture->GetWidth() / divisor, Texture->GetHeight() / divisor));
		SubTexture[SLIDER_HANDLE] = Ref<XYZ::SubTexture>::Create(Texture, glm::vec2(1, 2), glm::vec2(Texture->GetWidth() / divisor, Texture->GetHeight() / divisor));
		SubTexture[WINDOW] = Ref<XYZ::SubTexture>::Create(Texture, glm::vec2(0, 3), glm::vec2(Texture->GetWidth() / divisor, Texture->GetHeight() / divisor));
		SubTexture[MIN_BUTTON] = Ref<XYZ::SubTexture>::Create(Texture, glm::vec2(1, 3), glm::vec2(Texture->GetWidth() / divisor, Texture->GetHeight() / divisor));
		SubTexture[CLOSE_BUTTON] = Ref<XYZ::SubTexture>::Create(Texture, glm::vec2(2, 0), glm::vec2(Texture->GetWidth() / divisor, Texture->GetHeight() / divisor));
		SubTexture[DOWN_ARROW] = Ref<XYZ::SubTexture>::Create(Texture, glm::vec2(2, 3), glm::vec2(Texture->GetWidth() / divisor, Texture->GetHeight() / divisor));
		SubTexture[RIGHT_ARROW] = Ref<XYZ::SubTexture>::Create(Texture, glm::vec2(2, 2), glm::vec2(Texture->GetWidth() / divisor, Texture->GetHeight() / divisor));
		SubTexture[LEFT_ARROW] = Ref<XYZ::SubTexture>::Create(Texture, glm::vec2(3, 2), glm::vec2(Texture->GetWidth() / divisor, Texture->GetHeight() / divisor));
		SubTexture[DOCKSPACE] = Ref<XYZ::SubTexture>::Create(Texture, glm::vec2(0, 0), glm::vec2(Texture->GetWidth() / divisor, Texture->GetHeight() / divisor));


		Color[DEFAULT_COLOR] = { 1.0f, 1.0f, 1.0f, 1.0f };
		Color[HOOVER_COLOR] = { 1.0f, 2.5f, 2.8f, 1.0f };
		Color[SELECT_COLOR] = { 0.8f, 0.0f, 0.2f, 0.6f };
		Color[LINE_COLOR] = { 0.4f, 0.5f, 0.8f, 1.0f };
		Color[SELECTOR_COLOR] = { 1.0f, 1.0f, 1.0f, 1.0f };
	}
	InGuiDockNode::InGuiDockNode()
	{
		if (!s_FreeIDs.empty())
		{
			ID = s_FreeIDs.front();
			s_FreeIDs.pop();
		}
		else
			ID = s_NextID++;
	}
	InGuiDockNode::InGuiDockNode(uint32_t id)
	{
		if (s_NextID <= id)
			s_NextID = id + 1;
		ID = id;
	}
	InGuiDockNode::~InGuiDockNode()
	{
		s_FreeIDs.push(ID);
	}
}