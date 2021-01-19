#include "stdafx.h"
#include "InGui.h"

#include "XYZ/Core/Input.h"
#include "XYZ/Renderer/Renderer2D.h"

namespace XYZ {

	InGuiContext* InGui::s_Context = nullptr;

	static bool Collide(const glm::vec2& pos, const glm::vec2& size, const glm::vec2& point)
	{
		return (pos.x + size.x > point.x &&
			    pos.x		   < point.x&&
			    pos.y + size.y >  point.y &&
			    pos.y < point.y);
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
		auto [mx,my] = Input::GetMousePosition();

		s_Context->FrameData.ViewProjectionMatrix = viewProjectionMatrix;
		s_Context->FrameData.MousePosition = { mx, my };
	}

	void InGui::EndFrame()
	{
		XYZ_ASSERT(s_Context, "InGuiContext is not initialized");
		s_Context->FrameData.Flags = 0;

		Renderer2D::BeginScene(s_Context->FrameData.ViewProjectionMatrix);
		for (auto& mesh : s_Context->Meshes)
		{
			for (auto it = mesh.Quads.rbegin(); it != mesh.Quads.rend(); ++it)
			{
				Renderer2D::SubmitQuadNotCentered(it->Position, it->Size, it->TexCoord, it->TextureID, it->Color);
			}
			for (auto& line : mesh.Lines)
			{
				Renderer2D::SubmitLine(line.P0, line.P1, line.Color);
			}
		}
		Renderer2D::EndScene();
	}
	void InGui::OnEvent(Event& event)
	{
		if (event.GetEventType() == EventType::MouseButtonPressed)
		{
			s_Context->FrameData.Flags |= InGuiInputFlags::LeftClicked;
		}
		else if (event.GetEventType() == EventType::MouseButtonReleased)
		{
			s_Context->FrameData.Flags |= InGuiInputFlags::RightClicked;
		}
	}
	uint8_t InGui::Begin(uint32_t id, const char* name, const glm::vec2& position, const glm::vec2& size)
	{	
		if (s_Context->Windows.size() <= id)
		{
			s_Context->Windows.resize((size_t)id + 1);
			s_Context->Windows[id].Position = position;
			s_Context->Windows[id].Size = size;
			s_Context->Windows[id].ID = id;
			s_Context->Windows[id].Initialized = true;
		}
		s_Context->FrameData.ActiveWindowID = id;
		InGuiWindow& window = s_Context->Windows[id];

		uint8_t returnValue = 0;

		if (Collide(window.Position, window.Size, s_Context->FrameData.MousePosition))
		{
			returnValue |= InGuiReturnType::Hoovered;
			if (s_Context->FrameData.Flags & InGuiInputFlags::LeftClicked)
			{
				returnValue |= InGuiReturnType::Clicked;
				s_Context->FrameData.Flags &= ~InGuiInputFlags::LeftClicked;
			}
		}
		return returnValue;
	}

	InGuiRenderData::InGuiRenderData()
	{
		Ref<Shader> shader = Shader::Create("Assets/Shaders/InGuiShader.glsl");
		Texture = Texture2D::Create({ TextureWrap::Clamp, TextureParam::Linear, TextureParam::Nearest }, "Assets/Textures/Gui/TexturePack_Dark.png");
		Ref<Texture2D> colorPickerTexture = Texture2D::Create({ TextureWrap::Clamp, TextureParam::Nearest, TextureParam::Nearest }, "Assets/Textures/Gui/ColorPicker.png");

		Font = Ref<XYZ::Font>::Create(14, "Assets/Fonts/arial.ttf");
		Material = Ref<XYZ::Material>::Create(shader);
		Material->Set("u_Texture", Texture, TextureID);
		Material->Set("u_Texture", Font->GetTexture(), FontTextureID);
		Material->Set("u_Texture", colorPickerTexture, ColorPickerTextureID);
		Material->Set("u_ViewportSize", glm::vec2(Input::GetWindowSize().first, Input::GetWindowSize().second));


		float divisor = 8.0f;
		SubTexture[BUTTON]				= Ref<XYZ::SubTexture>::Create(Texture, glm::vec2(0, 0), glm::vec2(Texture->GetWidth() / divisor, Texture->GetHeight() / divisor));
		SubTexture[CHECKBOX_CHECKED]	= Ref<XYZ::SubTexture>::Create(Texture, glm::vec2(1, 1), glm::vec2(Texture->GetWidth() / divisor, Texture->GetHeight() / divisor));
		SubTexture[CHECKBOX_UNCHECKED]	= Ref<XYZ::SubTexture>::Create(Texture, glm::vec2(0, 1), glm::vec2(Texture->GetWidth() / divisor, Texture->GetHeight() / divisor));
		SubTexture[SLIDER]				= Ref<XYZ::SubTexture>::Create(Texture, glm::vec2(0, 0), glm::vec2(Texture->GetWidth() / divisor, Texture->GetHeight() / divisor));
		SubTexture[SLIDER_HANDLE]		= Ref<XYZ::SubTexture>::Create(Texture, glm::vec2(1, 2), glm::vec2(Texture->GetWidth() / divisor, Texture->GetHeight() / divisor));
		SubTexture[WINDOW]				= Ref<XYZ::SubTexture>::Create(Texture, glm::vec2(0, 3), glm::vec2(Texture->GetWidth() / divisor, Texture->GetHeight() / divisor));
		SubTexture[MIN_BUTTON]			= Ref<XYZ::SubTexture>::Create(Texture, glm::vec2(1, 3), glm::vec2(Texture->GetWidth() / divisor, Texture->GetHeight() / divisor));
		SubTexture[CLOSE_BUTTON]		= Ref<XYZ::SubTexture>::Create(Texture, glm::vec2(2, 0), glm::vec2(Texture->GetWidth() / divisor, Texture->GetHeight() / divisor));
		SubTexture[DOWN_ARROW]			= Ref<XYZ::SubTexture>::Create(Texture, glm::vec2(2, 3), glm::vec2(Texture->GetWidth() / divisor, Texture->GetHeight() / divisor));
		SubTexture[RIGHT_ARROW]			= Ref<XYZ::SubTexture>::Create(Texture, glm::vec2(2, 2), glm::vec2(Texture->GetWidth() / divisor, Texture->GetHeight() / divisor));
		SubTexture[LEFT_ARROW]			= Ref<XYZ::SubTexture>::Create(Texture, glm::vec2(3, 2), glm::vec2(Texture->GetWidth() / divisor, Texture->GetHeight() / divisor));
		SubTexture[DOCKSPACE]			= Ref<XYZ::SubTexture>::Create(Texture, glm::vec2(0, 0), glm::vec2(Texture->GetWidth() / divisor, Texture->GetHeight() / divisor));


		Color[DEFAULT_COLOR]  = { 1.0f,1.0f,1.0f,1.0f };
		Color[HOOVER_COLOR]   = { 1.0f, 2.5f, 2.8f, 1.0f };
		Color[SELECT_COLOR]   = { 0.8f,0.0f,0.2f,0.6f };
		Color[LINE_COLOR]     = { 0.4f,0.5f,0.8f,1.0f };
		Color[SELECTOR_COLOR] = { 1.0f,1.0f,1.0f,1.0f };
	}
}