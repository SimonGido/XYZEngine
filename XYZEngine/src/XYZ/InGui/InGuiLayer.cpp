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
		InGui::GetContext().m_Config.Texture = texture;
		InGui::GetContext().m_Config.Material->Set("u_Texture", texture, InGuiConfig::sc_DefaultTexture);
		InGui::GetContext().m_Config.Font = font;
		InGui::GetContext().m_Config.Material->Set("u_Texture", font->GetTexture(), InGuiConfig::sc_FontTexture);

		float divisor = 8.0f;
		Ref<SubTexture> windowSubTexture = Ref<XYZ::SubTexture>::Create(texture, glm::vec2(0, 3), glm::vec2(texture->GetWidth() / divisor, texture->GetHeight() / divisor));
		Ref<SubTexture> buttonSubTexture = Ref<XYZ::SubTexture>::Create(texture, glm::vec2(0, 0), glm::vec2(texture->GetWidth() / divisor, texture->GetHeight() / divisor));
		Ref<SubTexture> minimizeSubTexture = Ref<XYZ::SubTexture>::Create(texture, glm::vec2(1, 3), glm::vec2(texture->GetWidth() / divisor, texture->GetHeight() / divisor));
		Ref<SubTexture> checkedSubTexture = Ref<XYZ::SubTexture>::Create(texture, glm::vec2(1, 1), glm::vec2(texture->GetWidth() / divisor, texture->GetHeight() / divisor));
		Ref<SubTexture> unCheckedSubTexture = Ref<XYZ::SubTexture>::Create(texture, glm::vec2(0, 1), glm::vec2(texture->GetWidth() / divisor, texture->GetHeight() / divisor));
		Ref<SubTexture> sliderSubTexture = Ref<XYZ::SubTexture>::Create(texture, glm::vec2(0, 0), glm::vec2(texture->GetWidth() / divisor, texture->GetHeight() / divisor));
		Ref<SubTexture> handleSubTexture = Ref<XYZ::SubTexture>::Create(texture, glm::vec2(1, 2), glm::vec2(texture->GetWidth() / divisor, texture->GetHeight() / divisor));
		Ref<SubTexture> whiteSubTexture = Ref<XYZ::SubTexture>::Create(texture, glm::vec2(3, 0), glm::vec2(texture->GetWidth() / divisor, texture->GetHeight() / divisor));
		Ref<SubTexture> rightArrowSubTexture = Ref<XYZ::SubTexture>::Create(texture, glm::vec2(2, 2), glm::vec2(texture->GetWidth() / divisor, texture->GetHeight() / divisor));
		Ref<SubTexture> leftArrowSubTexture = Ref<XYZ::SubTexture>::Create(texture, glm::vec2(3, 2), glm::vec2(texture->GetWidth() / divisor, texture->GetHeight() / divisor));
		Ref<SubTexture> downArrowSubTexture = Ref<XYZ::SubTexture>::Create(texture, glm::vec2(2, 3), glm::vec2(texture->GetWidth() / divisor, texture->GetHeight() / divisor));
		Ref<SubTexture> pauseSubTexture = Ref<XYZ::SubTexture>::Create(texture, glm::vec2(2, 1), glm::vec2(texture->GetWidth() / divisor, texture->GetHeight() / divisor));

		InGui::GetContext().m_Config.SubTextures[InGuiConfig::Window]			 = windowSubTexture;
		InGui::GetContext().m_Config.SubTextures[InGuiConfig::Button]			 = buttonSubTexture;
		InGui::GetContext().m_Config.SubTextures[InGuiConfig::MinimizeButton]	 = minimizeSubTexture;
		InGui::GetContext().m_Config.SubTextures[InGuiConfig::CheckboxChecked]	 = checkedSubTexture;
		InGui::GetContext().m_Config.SubTextures[InGuiConfig::CheckboxUnChecked] = unCheckedSubTexture;
		InGui::GetContext().m_Config.SubTextures[InGuiConfig::Slider]			 = sliderSubTexture;
		InGui::GetContext().m_Config.SubTextures[InGuiConfig::SliderHandle]		 = handleSubTexture;
		InGui::GetContext().m_Config.SubTextures[InGuiConfig::White]			 = whiteSubTexture;
		InGui::GetContext().m_Config.SubTextures[InGuiConfig::RightArrow]		 = rightArrowSubTexture;
		InGui::GetContext().m_Config.SubTextures[InGuiConfig::LeftArrow]		 = leftArrowSubTexture;
		InGui::GetContext().m_Config.SubTextures[InGuiConfig::DownArrow]		 = downArrowSubTexture;
		InGui::GetContext().m_Config.SubTextures[InGuiConfig::Pause]			 = pauseSubTexture;
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


		InGui::Begin("Havko",
			  InGuiWindowStyleFlags::MenuEnabled
			| InGuiWindowStyleFlags::PanelEnabled
			| InGuiWindowStyleFlags::ScrollEnabled
			| InGuiWindowStyleFlags::LabelEnabled
		);


		if (InGui::BeginMenuBar())
		{
			if (IS_SET(InGui::BeginMenu("Test label", 70.0f), InGui::Pressed))
			{
				InGui::MenuItem("Testa asgas");
				InGui::MenuItem("Testa kra kra");
				InGui::MenuItem("Testa kra aas");
				InGui::MenuItem("Testa kra ka");
				InGui::MenuItem("Testa kra kraa");
				InGui::MenuItem("Testa kra a");
				InGui::MenuItem("Testa kra as");
				InGui::MenuItem("Testa kra ka");
			}
			InGui::EndMenu();

			if (IS_SET(InGui::BeginMenu("Haga label", 70.0f), InGui::Pressed))
			{
				InGui::MenuItem("T asgas");
				InGui::MenuItem("T kra kra");
				InGui::MenuItem("T kra aas");
				InGui::MenuItem("T kra ka");
				InGui::MenuItem("T kra kraa");
				InGui::MenuItem("T kra a");
				InGui::MenuItem("T kra as");
				InGui::MenuItem("T kra ka");
			}
			InGui::EndMenu();


			InGui::EndMenuBar();
		}

		if (InGui::BeginTab("First Tab"))
		{
			static bool checked = false;
			InGui::Checkbox("Hopbiasf", { 25.0f,25.0f }, checked);
			if (checked)
			{
				std::cout << "Checked" << std::endl;
			}
		
			if (IS_SET(InGui::Button("Opica hah assa", { 75.0f, 25.0f }), InGui::Pressed))
			{
				std::cout << "Pressed" << std::endl;
			}
		
			static float value = 5.0f;
			if (IS_SET(InGui::SliderFloat("Slider example", { 150.0f, 25.0f }, value, 0.0f, 10.0f), InGui::Pressed))
			{
				std::cout << "Value changed " << value << std::endl;
			}
			static float vvalue = 5.0f;
			if (IS_SET(InGui::VSliderFloat("VSlider example", { 25.0f, 150.0f }, vvalue, 0.0f, 10.0f), InGui::Pressed))
			{
				std::cout << "Value changed " << value << std::endl;
			}
		
			static float values[4] = { 2.0f,2.5f, 1.0f, 4.0f };
			InGui::Float2("Hava1", "Hava2", glm::vec2(40.0f, 30.0f), values, 2);
			InGui::Float3("##Haafsva1", "Havasfaa2", "Hhoho", glm::vec2(40.0f, 30.0f), values, 2);
			InGui::Float4("###Haasgva1", "Hagsdgva2", "hasfs", "Hasdasasd", glm::vec2(40.0f, 30.0f), values, 2);
			InGui::Float("test", glm::vec2(40.0f, 30.0f), value, 2);
		}
		InGui::EndTab();
		
		if (InGui::BeginTab("Second Tab"))
		{
			static bool checked = false;
			InGui::Checkbox("Hopbiadsgassf", { 25.0f,25.0f }, checked);
			if (checked)
			{
				std::cout << "Checked" << std::endl;
			}
		}
		InGui::EndTab();
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