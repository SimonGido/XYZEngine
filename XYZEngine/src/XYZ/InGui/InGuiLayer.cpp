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
		InGui::Init("InGui.yaml");

		uint32_t whiteTextureData = 0xffffffff;
		Ref<Texture2D> whiteTexture = Texture2D::Create(1, 1, 4, { TextureWrap::Clamp, TextureParam::Linear, TextureParam::Nearest });
		whiteTexture->SetData(&whiteTextureData, sizeof(uint32_t));
		Ref<Texture2D> texture = Texture2D::Create({ TextureWrap::Clamp, TextureParam::Linear, TextureParam::Nearest }, "Assets/Textures/Gui/ingui.png");
		Ref<Font> font = Ref<XYZ::Font>::Create(14, "Assets/Fonts/arial.ttf");
		
		InGuiConfig& config = InGui::GetContext().m_Config;
		config.Texture = texture;
		config.Material->Set("u_Texture", texture, InGuiConfig::DefaultTextureIndex);
		config.WhiteTexture = whiteTexture;
		config.Material->Set("u_Texture", whiteTexture, InGuiConfig::WhiteTextureIndex);	
		config.Font = font;
		config.Material->Set("u_Texture", font->GetTexture(), InGuiConfig::FontTextureIndex);

		float divisor = 4.0f;
		
		config.SubTextures[InGuiConfig::MinimizeButton]	     = Ref<XYZ::SubTexture>::Create(texture, glm::vec2(2, 2), glm::vec2(texture->GetWidth() / divisor, texture->GetHeight() / divisor));
		config.SubTextures[InGuiConfig::CheckboxChecked]	 = Ref<XYZ::SubTexture>::Create(texture, glm::vec2(1, 2), glm::vec2(texture->GetWidth() / divisor, texture->GetHeight() / divisor));
		config.SubTextures[InGuiConfig::CheckboxUnChecked]   = Ref<XYZ::SubTexture>::Create(texture, glm::vec2(0, 2), glm::vec2(texture->GetWidth() / divisor, texture->GetHeight() / divisor));
		config.SubTextures[InGuiConfig::RightArrow]			 = Ref<XYZ::SubTexture>::Create(texture, glm::vec2(0, 3), glm::vec2(texture->GetWidth() / divisor, texture->GetHeight() / divisor));
		config.SubTextures[InGuiConfig::LeftArrow]			 = Ref<XYZ::SubTexture>::Create(texture, glm::vec2(2, 3), glm::vec2(texture->GetWidth() / divisor, texture->GetHeight() / divisor));
		config.SubTextures[InGuiConfig::DownArrow]			 = Ref<XYZ::SubTexture>::Create(texture, glm::vec2(1, 3), glm::vec2(texture->GetWidth() / divisor, texture->GetHeight() / divisor));
		config.SubTextures[InGuiConfig::Pause]				 = Ref<XYZ::SubTexture>::Create(texture, glm::vec2(3, 3), glm::vec2(texture->GetWidth() / divisor, texture->GetHeight() / divisor));
		config.WhiteSubTexture								 = Ref<XYZ::SubTexture>::Create(whiteTexture, glm::vec2(0.0f));
	}

	void InGuiLayer::OnDetach()
	{
		InGui::Shutdown();
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
		InGui::Begin("Havko",
			  InGuiWindowStyleFlags::MenuEnabled
			| InGuiWindowStyleFlags::PanelEnabled
			| InGuiWindowStyleFlags::ScrollEnabled
			| InGuiWindowStyleFlags::LabelEnabled
		);


		InGui::DockSpace();

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
		
		if (InGui::BeginTab("Second Tab"))
		{
			static bool checked = false;
			InGui::Checkbox("Hopbiadsgassf", { 25.0f,25.0f }, checked);
			if (checked)
			{
				std::cout << "Checked" << std::endl;
			}
		}

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