#include "stdafx.h"
#include "InGui.h"

#include "XYZ/Core/Input.h"
#include "XYZ/Event/InputEvent.h"
#include "XYZ/Renderer/Renderer2D.h"
#include "XYZ/Renderer/Renderer.h"

#include "InGuiFactory.h"

#include <glm/gtx/transform.hpp>

namespace XYZ {

	InGuiContext* InGui::s_Context = nullptr;

	static bool Collide(const glm::vec2& pos, const glm::vec2& size, const glm::vec2& point)
	{
		return (pos.x + size.x > point.x &&
			    pos.x		   < point.x&&
			    pos.y + size.y >  point.y &&
			    pos.y < point.y);
	}

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

	void InGui::Init()
	{
		s_Context = new InGuiContext();
	}

	void InGui::Destroy()
	{
		if (s_Context)
			delete s_Context;
	}

	void InGui::BeginFrame(const glm::mat4& viewProjectionMatrix)
	{
		XYZ_ASSERT(s_Context, "InGuiContext is not initialized");
		s_Context->FrameData.ViewProjectionMatrix = viewProjectionMatrix;	
	}

	void InGui::EndFrame()
	{
		XYZ_ASSERT(s_Context, "InGuiContext is not initialized");
		s_Context->FrameData.Flags = 0;

		glm::mat4 viewMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f));
		viewMatrix = glm::inverse(viewMatrix);

		Renderer2D::BeginScene(s_Context->FrameData.ViewProjectionMatrix * viewMatrix);
		Renderer2D::SetMaterial(s_Context->RenderData.Material);
		
		for (auto winIt = s_Context->Windows.rbegin(); winIt != s_Context->Windows.rend(); ++winIt)
		{
			for (auto it = winIt->Mesh.Quads.rbegin(); it != winIt->Mesh.Quads.rend(); ++it)
			{
				Renderer2D::SubmitQuadNotCentered(it->Position, it->Size, it->TexCoord, it->TextureID, it->Color);
			}
			for (auto& line : winIt->Mesh.Lines)
			{
				Renderer2D::SubmitLine(line.P0, line.P1, line.Color);
			}
		}
		Renderer2D::Flush();
		Renderer2D::EndScene();
		Renderer::WaitAndRender();

		handleWindowMove();
	}


	void InGui::OnEvent(Event& event)
	{
		if (event.GetEventType() == EventType::MouseButtonPressed)
		{
			MouseButtonPressEvent& e = (MouseButtonPressEvent&)event;
			if (e.IsButtonPressed(MouseCode::XYZ_MOUSE_BUTTON_LEFT))
			{
				for (auto& window : s_Context->Windows)
				{
					if (window.Flags & InGuiWindowFlags::Initialized
						&& Collide(window.Position, window.Size, s_Context->FrameData.MousePosition))
					{
						s_Context->FrameData.Flags |= InGuiInputFlags::LeftClicked;
						s_Context->FrameData.MovedWindowID = window.ID;
						s_Context->FrameData.MouseOffset = s_Context->FrameData.MousePosition - window.Position;
						if (window.Flags & InGuiWindowFlags::EventBlocking)
							event.Handled = true;
						return;
					}
				}
			}
		}
		else if (event.GetEventType() == EventType::MouseButtonReleased)
		{
			MouseButtonReleaseEvent& e = (MouseButtonReleaseEvent&)event;
			if (e.IsButtonReleased(MouseCode::XYZ_MOUSE_BUTTON_LEFT))
			{
				s_Context->FrameData.MovedWindowID = InGuiFrameData::NullID;
				for (auto& window : s_Context->Windows)
				{
					if (window.Flags & InGuiWindowFlags::Initialized
						&& Collide(window.Position, window.Size, s_Context->FrameData.MousePosition))
					{				
						if (window.Flags & InGuiWindowFlags::EventBlocking)
							event.Handled = true;

						return;
					}
				}
			}
		}
		else if (event.GetEventType() == EventType::MouseMoved)
		{
			MouseMovedEvent& e = (MouseMovedEvent&)event;
			s_Context->FrameData.MousePosition = { (float)e.GetX(), (float)e.GetY() };
		}
	}

	InGuiWindow& InGui::getInitializedWindow(uint32_t id, const glm::vec2& position, const glm::vec2& size)
	{
		if (s_Context->Windows.size() <= id)
		{
			s_Context->Windows.resize((size_t)id + 1);
		}
		if (!(s_Context->Windows[id].Flags & InGuiWindowFlags::Initialized))
		{
			s_Context->Windows[id].Position = position;
			s_Context->Windows[id].Size = size;
			s_Context->Windows[id].ID = id;
			s_Context->Windows[id].Flags |= (InGuiWindowFlags::Initialized | InGuiWindowFlags::EventBlocking);
		}
		return s_Context->Windows[id];
	}

	void InGui::handleWindowMove()
	{
		uint32_t id = s_Context->FrameData.MovedWindowID;
		if (id != InGuiFrameData::NullID)
		{
			s_Context->Windows[id].Position = s_Context->FrameData.MousePosition - s_Context->FrameData.MouseOffset;
		}
	}

	uint8_t InGui::Begin(uint32_t id, const char* name, const glm::vec2& position, const glm::vec2& size)
	{	
		XYZ_ASSERT(s_Context->FrameData.ActiveWindowID == InGuiFrameData::NullID, "Missing end call");
		s_Context->FrameData.ActiveWindowID = id;
		InGuiWindow& window = getInitializedWindow(id, position, size);

		uint8_t returnValue = 0;
		glm::vec4 color = s_Context->RenderData.Color[InGuiRenderData::DEFAULT_COLOR];
		if (Collide(window.Position, window.Size, s_Context->FrameData.MousePosition))
		{
			returnValue |= InGuiReturnType::Hoovered;
			color = s_Context->RenderData.Color[InGuiRenderData::HOOVER_COLOR];
			if (s_Context->FrameData.Flags & InGuiInputFlags::LeftClicked)
			{
				returnValue |= InGuiReturnType::Clicked;
				s_Context->FrameData.Flags &= ~InGuiInputFlags::LeftClicked;
			}
		}
		InGuiFactory::GenerateWindow(name, window, color, s_Context->RenderData);
		return returnValue;
	}

	void InGui::End()
	{
		XYZ_ASSERT(s_Context->FrameData.ActiveWindowID != InGuiFrameData::NullID, "Missing begin call");
		s_Context->FrameData.ActiveWindowID = InGuiFrameData::NullID;
	}

}