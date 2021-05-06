#include "stdafx.h"
#include "BasicUILayer.h"

#include "BasicUI.h"

namespace XYZ {
	void bUILayer::OnAttach()
	{
		bUI::Init();

		Ref<Texture2D> texture = Texture2D::Create({ TextureWrap::Clamp, TextureParam::Linear, TextureParam::Nearest }, "Assets/Textures/Gui/TexturePack_Dark.png");	
		Ref<Font> font = Ref<XYZ::Font>::Create(14, "Assets/Fonts/arial.ttf");
		bUI::GetConfig().SetTexture(texture);
		bUI::GetConfig().SetFont(font);

		float divisor = 8.0f;
		Ref<SubTexture> buttonSubTexture		= Ref<XYZ::SubTexture>::Create(texture, glm::vec2(0, 0), glm::vec2(texture->GetWidth() / divisor, texture->GetHeight() / divisor));		
		Ref<SubTexture> minimizeSubTexture		= Ref<XYZ::SubTexture>::Create(texture, glm::vec2(1, 3), glm::vec2(texture->GetWidth() / divisor, texture->GetHeight() / divisor));
		Ref<SubTexture> checkedSubTexture		= Ref<XYZ::SubTexture>::Create(texture, glm::vec2(1, 1), glm::vec2(texture->GetWidth() / divisor, texture->GetHeight() / divisor));
		Ref<SubTexture> unCheckedSubTexture		= Ref<XYZ::SubTexture>::Create(texture, glm::vec2(0, 1), glm::vec2(texture->GetWidth() / divisor, texture->GetHeight() / divisor));
		Ref<SubTexture> sliderSubTexture		= Ref<XYZ::SubTexture>::Create(texture, glm::vec2(0, 0), glm::vec2(texture->GetWidth() / divisor, texture->GetHeight() / divisor));
		Ref<SubTexture> handleSubTexture		= Ref<XYZ::SubTexture>::Create(texture, glm::vec2(1, 2), glm::vec2(texture->GetWidth() / divisor, texture->GetHeight() / divisor));
		Ref<SubTexture> whiteSubTexture			= Ref<XYZ::SubTexture>::Create(texture, glm::vec2(3, 0), glm::vec2(texture->GetWidth() / divisor, texture->GetHeight() / divisor));
		Ref<SubTexture> rightArrowSubTexture	= Ref<XYZ::SubTexture>::Create(texture, glm::vec2(2, 2), glm::vec2(texture->GetWidth() / divisor, texture->GetHeight() / divisor));
		Ref<SubTexture> downArrowSubTexture		= Ref<XYZ::SubTexture>::Create(texture, glm::vec2(2, 3), glm::vec2(texture->GetWidth() / divisor, texture->GetHeight() / divisor));
		Ref<SubTexture> pauseSubTexture			= Ref<XYZ::SubTexture>::Create(texture, glm::vec2(2, 1), glm::vec2(texture->GetWidth() / divisor, texture->GetHeight() / divisor));

		bUI::GetConfig().SetSubTexture(buttonSubTexture, bUIConfig::Button);
		bUI::GetConfig().SetSubTexture(minimizeSubTexture, bUIConfig::MinimizeButton);
		bUI::GetConfig().SetSubTexture(checkedSubTexture, bUIConfig::CheckboxChecked);
		bUI::GetConfig().SetSubTexture(unCheckedSubTexture, bUIConfig::CheckboxUnChecked);
		bUI::GetConfig().SetSubTexture(sliderSubTexture, bUIConfig::Slider);
		bUI::GetConfig().SetSubTexture(handleSubTexture, bUIConfig::SliderHandle);
		bUI::GetConfig().SetSubTexture(whiteSubTexture, bUIConfig::White);
		bUI::GetConfig().SetSubTexture(rightArrowSubTexture, bUIConfig::RightArrow);
		bUI::GetConfig().SetSubTexture(downArrowSubTexture, bUIConfig::DownArrow);
		bUI::GetConfig().SetSubTexture(pauseSubTexture, bUIConfig::Pause);

	}
	void bUILayer::OnDetach()
	{
		bUI::Shutdown();
	}
	void bUILayer::OnUpdate(Timestep ts)
	{
		bUI::Update();
	}
	void bUILayer::OnEvent(Event& event)
	{
		bUI::OnEvent(event);
	}
}