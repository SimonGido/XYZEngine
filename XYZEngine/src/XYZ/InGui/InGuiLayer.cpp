#include "stdafx.h"
#include "InGuiLayer.h"

#include "InGui.h"

#include "XYZ/Renderer/Texture.h"
#include "XYZ/Renderer/Material.h"
#include "XYZ/Renderer/Font.h"
#include "XYZ/Renderer/SubTexture.h"
#include "XYZ/Core/Input.h"

namespace XYZ {
	void InGuiLayer::OnAttach()
	{
		Ref<Texture2D> texture = Texture2D::Create({ TextureWrap::Clamp, TextureParam::Linear, TextureParam::Nearest }, "Assets/Textures/Gui/TexturePack_Dark.png");
		Ref<Font> font = Ref<XYZ::Font>::Create(14, "Assets/Fonts/arial.ttf");
		InGui::GetContext().GetConfig().SetTexture(texture);
		InGui::GetContext().GetConfig().SetFont(font);

		float divisor = 8.0f;
		Ref<SubTexture> buttonSubTexture = Ref<XYZ::SubTexture>::Create(texture, glm::vec2(0, 0), glm::vec2(texture->GetWidth() / divisor, texture->GetHeight() / divisor));
		Ref<SubTexture> minimizeSubTexture = Ref<XYZ::SubTexture>::Create(texture, glm::vec2(1, 3), glm::vec2(texture->GetWidth() / divisor, texture->GetHeight() / divisor));
		Ref<SubTexture> checkedSubTexture = Ref<XYZ::SubTexture>::Create(texture, glm::vec2(1, 1), glm::vec2(texture->GetWidth() / divisor, texture->GetHeight() / divisor));
		Ref<SubTexture> unCheckedSubTexture = Ref<XYZ::SubTexture>::Create(texture, glm::vec2(0, 1), glm::vec2(texture->GetWidth() / divisor, texture->GetHeight() / divisor));
		Ref<SubTexture> sliderSubTexture = Ref<XYZ::SubTexture>::Create(texture, glm::vec2(0, 0), glm::vec2(texture->GetWidth() / divisor, texture->GetHeight() / divisor));
		Ref<SubTexture> handleSubTexture = Ref<XYZ::SubTexture>::Create(texture, glm::vec2(1, 2), glm::vec2(texture->GetWidth() / divisor, texture->GetHeight() / divisor));
		Ref<SubTexture> whiteSubTexture = Ref<XYZ::SubTexture>::Create(texture, glm::vec2(3, 0), glm::vec2(texture->GetWidth() / divisor, texture->GetHeight() / divisor));
		Ref<SubTexture> rightArrowSubTexture = Ref<XYZ::SubTexture>::Create(texture, glm::vec2(2, 2), glm::vec2(texture->GetWidth() / divisor, texture->GetHeight() / divisor));
		Ref<SubTexture> downArrowSubTexture = Ref<XYZ::SubTexture>::Create(texture, glm::vec2(2, 3), glm::vec2(texture->GetWidth() / divisor, texture->GetHeight() / divisor));
		Ref<SubTexture> pauseSubTexture = Ref<XYZ::SubTexture>::Create(texture, glm::vec2(2, 1), glm::vec2(texture->GetWidth() / divisor, texture->GetHeight() / divisor));

		InGui::GetContext().GetConfig().SetSubTexture(buttonSubTexture,		InGuiConfig::Window);
		InGui::GetContext().GetConfig().SetSubTexture(buttonSubTexture,     InGuiConfig::Button);
		InGui::GetContext().GetConfig().SetSubTexture(minimizeSubTexture,   InGuiConfig::MinimizeButton);
		InGui::GetContext().GetConfig().SetSubTexture(checkedSubTexture,    InGuiConfig::CheckboxChecked);
		InGui::GetContext().GetConfig().SetSubTexture(unCheckedSubTexture,  InGuiConfig::CheckboxUnChecked);
		InGui::GetContext().GetConfig().SetSubTexture(sliderSubTexture,     InGuiConfig::Slider);
		InGui::GetContext().GetConfig().SetSubTexture(handleSubTexture,     InGuiConfig::SliderHandle);
		InGui::GetContext().GetConfig().SetSubTexture(whiteSubTexture,      InGuiConfig::White);
		InGui::GetContext().GetConfig().SetSubTexture(rightArrowSubTexture, InGuiConfig::RightArrow);
		InGui::GetContext().GetConfig().SetSubTexture(downArrowSubTexture,  InGuiConfig::DownArrow);
		InGui::GetContext().GetConfig().SetSubTexture(pauseSubTexture,      InGuiConfig::Pause);
	}

	void InGuiLayer::OnDetach()
	{
	}

	void InGuiLayer::OnUpdate(Timestep ts)
	{
		
	}

	void InGuiLayer::OnEvent(Event& event)
	{
		InGui::GetContext().OnEvent(event);
	}
	void InGuiLayer::OnInGuiRender()
	{
		InGui::Begin("Test");


		InGui::End();


		InGui::Begin("Havko");


		InGui::End();
	}
	void InGuiLayer::Begin()
	{
		InGui::BeginFrame();
		auto [w, h] = Input::GetWindowSize();
		InGui::GetContext().SetViewportSize(w, h);
	}
	void InGuiLayer::End()
	{
		InGui::EndFrame();
	}
}