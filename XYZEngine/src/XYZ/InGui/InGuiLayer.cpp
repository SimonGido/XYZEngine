#include "stdafx.h"
#include "InGuiLayer.h"

#include "InGuiCore.h"

#include "XYZ/Core/Input.h"
#include "XYZ/Core/MouseCodes.h"
#include "XYZ/Core/KeyCodes.h"
namespace XYZ {
	void InGuiLayer::OnAttach()
	{
		Ref<Shader> shader = Shader::Create("Assets/Shaders/InGuiShader.glsl");
		Ref<Texture2D> texture = Texture2D::Create(TextureWrap::Clamp, "Assets/Textures/Gui/TexturePack.png");
		Ref<Texture2D> colorPickerTexture = Texture2D::Create(TextureWrap::Clamp, "Assets/Textures/Gui/ColorPicker.png");
		m_Material = Material::Create(shader);

		uint32_t textureID = 0;
		uint32_t fontTextureID = 1;
		uint32_t colorPickerTextureID = 2;

		m_Material->Set("u_Texture", texture, textureID);
		m_Material->Set("u_Texture", Texture2D::Create(TextureWrap::Clamp, "Assets/Font/Arial.png"), fontTextureID);
		m_Material->Set("u_Texture", colorPickerTexture, colorPickerTextureID);
		m_Material->Set("u_ViewportSize", glm::vec2(Input::GetWindowSize().first, Input::GetWindowSize().second));
		InGui::Init({
			m_Material,
			texture,
			colorPickerTexture,
			Ref<Font>::Create("Assets/Font/Arial.fnt"),
			Ref<SubTexture2D>::Create(texture, glm::vec2(0, 0), glm::vec2(texture->GetWidth() / 4, texture->GetHeight() / 4)),
			Ref<SubTexture2D>::Create(texture, glm::vec2(1, 1), glm::vec2(texture->GetWidth() / 4, texture->GetHeight() / 4)),
			Ref<SubTexture2D>::Create(texture, glm::vec2(0, 1), glm::vec2(texture->GetWidth() / 4, texture->GetHeight() / 4)),
			Ref<SubTexture2D>::Create(texture, glm::vec2(0, 0), glm::vec2(texture->GetWidth() / 4, texture->GetHeight() / 4)),
			Ref<SubTexture2D>::Create(texture, glm::vec2(1, 2), glm::vec2(texture->GetWidth() / 4, texture->GetHeight() / 4)),
			Ref<SubTexture2D>::Create(texture, glm::vec2(0, 3), glm::vec2(texture->GetWidth() / 4, texture->GetHeight() / 4)),
			Ref<SubTexture2D>::Create(texture, glm::vec2(2, 3), glm::vec2(texture->GetWidth() / 4, texture->GetHeight() / 4)),
			Ref<SubTexture2D>::Create(texture, glm::vec2(0, 0), glm::vec2(texture->GetWidth() / 4, texture->GetHeight() / 4)),
			textureID,
			fontTextureID,
			colorPickerTextureID,
		},
		{
			{1.0f,1.0f,1.0f,1.0f},
			{0.8f,0.0f,0.2f,1.0f},
			{0.8f,0.0f,0.2f,0.6f},
			{0.7f,0.7f},
			1000.f
		});	

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
		EventDispatcher dispatcher(event);
		if (dispatcher.Dispatch<MouseButtonPressEvent>(Hook(&InGuiLayer::onMouseButtonPress, this)))
		{}
		else if (dispatcher.Dispatch<MouseButtonReleaseEvent>(Hook(&InGuiLayer::onMouseButtonRelease,this)))
		{}
		else if (dispatcher.Dispatch<MouseMovedEvent>(Hook(&InGuiLayer::onMouseMove, this)))
		{}
		else if (dispatcher.Dispatch<WindowResizeEvent>(Hook(&InGuiLayer::onWindowResize,this)))
		{}
		else if (dispatcher.Dispatch<KeyPressedEvent>(Hook(&InGuiLayer::onKeyPressed, this)))
		{}
	}
	void InGuiLayer::Begin()
	{
		InGui::BeginFrame();
	}
	void InGuiLayer::End()
	{
		InGui::EndFrame();
	}
	bool InGuiLayer::onMouseButtonPress(MouseButtonPressEvent& e)
	{
		if (e.GetButton() == MouseCode::XYZ_MOUSE_BUTTON_LEFT)
		{
			InGui::OnLeftMouseButtonPress();
		}
		else if (e.GetButton() == MouseCode::XYZ_MOUSE_BUTTON_RIGHT)
		{
			InGui::OnRightMouseButtonPress();
		}

		return false;
	}
	bool InGuiLayer::onMouseButtonRelease(MouseButtonReleaseEvent& e)
	{
		if (e.GetButton() == MouseCode::XYZ_MOUSE_BUTTON_LEFT)
		{
			InGui::OnLeftMouseButtonRelease();
		}
		else if (e.GetButton() == MouseCode::XYZ_MOUSE_BUTTON_RIGHT)
		{
			InGui::OnRightMouseButtonRelease();
		}

		return false;
	}
	bool InGuiLayer::onMouseMove(MouseMovedEvent& e)
	{
		InGui::OnMouseMove({ e.GetX(),e.GetY() });
		return false;
	}
	bool InGuiLayer::onWindowResize(WindowResizeEvent& e)
	{
		InGui::OnWindowResize({ e.GetWidth(),e.GetHeight() });

		m_Material->Set("u_ViewportSize", glm::vec2(e.GetWidth(), e.GetHeight()));
		return false;
	}
	bool InGuiLayer::onKeyPressed(KeyPressedEvent& e)
	{
		InGui::OnKeyPressed(e.GetKey(), e.GetMod());
		return false;
	}
}
