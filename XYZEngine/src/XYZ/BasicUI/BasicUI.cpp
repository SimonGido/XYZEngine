#include "stdafx.h"
#include "BasicUI.h"

#include "XYZ/Renderer/Renderer2D.h"
#include "XYZ/Renderer/Renderer.h"
#include "XYZ/Core/Application.h"
#include "XYZ/Core/Input.h"

#include <glm/gtx/transform.hpp>

namespace XYZ {

	static bUIContext* s_Context = nullptr;

	void bUI::Init()
	{
		s_Context = new bUIContext();
		auto& app = Application::Get();
		s_Context->ViewportSize = glm::vec2(app.GetWindow().GetWidth(), 
											app.GetWindow().GetHeight());
	}
	void bUI::Shutdown()
	{
		delete s_Context;
	}
	void bUI::Update()
	{
		s_Context->Renderer.Begin();
		for (size_t i = 0; i < s_Context->Data.Size(); ++i)
		{
			s_Context->Data.GetElement<bUIElement>(i)->PushQuads(s_Context->Renderer);
		}
		glm::mat4 viewMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f));
		viewMatrix = glm::inverse(viewMatrix);
		Renderer2D::BeginScene(glm::ortho(0.0f, s_Context->ViewportSize.x, s_Context->ViewportSize.y, 0.0f) * viewMatrix);
		Renderer2D::SetMaterial(s_Context->Config.m_Material);
		for (const bUIQuad& quad : s_Context->Renderer.GetMesh().Quads)
			Renderer2D::SubmitQuadNotCentered(quad.Position, quad.Size, quad.TexCoord, quad.TextureID, quad.Color);

		Renderer2D::Flush();
		Renderer2D::EndScene();
		Renderer::WaitAndRender();
	}
	void bUI::OnEvent(Event& event)
	{
		EventDispatcher dispatcher(event);
		dispatcher.Dispatch<WindowResizeEvent>(&onWindowResize);
		dispatcher.Dispatch<MouseButtonPressEvent>(&onMouseButtonPress);
		dispatcher.Dispatch<MouseButtonReleaseEvent>(&onMouseButtonRelease);
		dispatcher.Dispatch<MouseMovedEvent>(&onMouseMove);
	}
	bUIConfig& bUI::GetConfig()
	{
		return s_Context->Config;
	}
	const bUIContext& bUI::GetContext()
	{
		return *s_Context;
	}
	bool bUI::onWindowResize(WindowResizeEvent& event)
	{
		glm::vec2 newViewportSize((float)event.GetWidth(), (float)event.GetHeight());
		s_Context->ViewportSize = newViewportSize;
		return false;
	}
	bool bUI::onMouseButtonPress(MouseButtonPressEvent& event)
	{
		auto [mx, my] = Input::GetMousePosition();
		glm::vec2 mousePosition = { mx , my };
		if (event.IsButtonPressed(MouseCode::MOUSE_BUTTON_LEFT))
		{
			for (size_t i = 0; i < s_Context->Data.Size(); ++i)
			{
				if (s_Context->Data.GetElement<bUIElement>(i)->OnLeftMousePressed(mousePosition))
					return true;
			}
		}
		else if (event.IsButtonPressed(MouseCode::MOUSE_BUTTON_RIGHT))
		{
			for (size_t i = 0; i < s_Context->Data.Size(); ++i)
			{
				bUIElement* element = s_Context->Data.GetElement<bUIElement>(i);
				if (element->OnRightMousePressed(mousePosition))
				{
					s_Context->EditData.Element = element;
					s_Context->EditData.MouseOffset = mousePosition - element->Coords;
					return true;
				}
			}
		}
		return false;
	}
	bool bUI::onMouseButtonRelease(MouseButtonReleaseEvent& event)
	{
		if (event.IsButtonReleased(MouseCode::MOUSE_BUTTON_RIGHT))
		{
			s_Context->EditData.Element = nullptr;
		}
		return false;
	}
	bool bUI::onMouseMove(MouseMovedEvent& event)
	{
		auto [mx, my] = Input::GetMousePosition();
		glm::vec2 mousePosition = { mx , my };
		if (s_Context->EditData.Element)
		{
			s_Context->EditData.Element->Coords = mousePosition - s_Context->EditData.MouseOffset;
		}
		for (size_t i = 0; i < s_Context->Data.Size(); ++i)
		{
			if (s_Context->Data.GetElement<bUIElement>(i)->OnMouseMoved(mousePosition))
				return true;
		}
		return false;
	}
	bUIContext& bUI::getContext()
	{
		return *s_Context;
	}
}