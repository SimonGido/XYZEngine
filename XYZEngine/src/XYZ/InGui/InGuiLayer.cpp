#include "stdafx.h"
#include "InGuiLayer.h"

#include "InGui.h"

#include "XYZ/Core/Input.h"
#include "XYZ/Core/MouseCodes.h"
#include "XYZ/Core/KeyCodes.h"


namespace XYZ {
	void InGuiLayer::OnAttach()
	{	
		InGui::Init({});
	}
	void InGuiLayer::OnDetach()
	{
		InGui::Destroy();
	}
	void InGuiLayer::OnUpdate(Timestep ts)
	{
		
	}
	void InGuiLayer::OnEvent(Event& event)
	{
		EventDispatcher dispatcher(event);
		
		if (dispatcher.Dispatch<MouseButtonPressEvent>(Hook(&InGuiLayer::onMouseButtonPress, this)))
		{
		}
		else if (dispatcher.Dispatch<MouseButtonReleaseEvent>(Hook(&InGuiLayer::onMouseButtonRelease, this)))
		{
		}
		else if (dispatcher.Dispatch<MouseMovedEvent>(Hook(&InGuiLayer::onMouseMove, this)))
		{
		}
		else if (dispatcher.Dispatch<WindowResizeEvent>(Hook(&InGuiLayer::onWindowResize, this)))
		{
		}
		else if (dispatcher.Dispatch<KeyPressedEvent>(Hook(&InGuiLayer::onKeyPressed, this)))
		{
		}
		else if (dispatcher.Dispatch<MouseScrollEvent>(Hook(&InGuiLayer::onMouseScroll, this)))
		{

		}
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
		if (e.IsButtonPressed(MouseCode::XYZ_MOUSE_BUTTON_LEFT))
		{
			return InGui::OnLeftMouseButtonPress();
		}
		else if (e.IsButtonPressed(MouseCode::XYZ_MOUSE_BUTTON_RIGHT))
		{
			return InGui::OnRightMouseButtonPress();
		}

		return false;
	}
	bool InGuiLayer::onMouseButtonRelease(MouseButtonReleaseEvent& e)
	{
		if (e.IsButtonReleased(MouseCode::XYZ_MOUSE_BUTTON_LEFT))
		{
			return InGui::OnLeftMouseButtonRelease();
		}
		else if (e.IsButtonReleased(MouseCode::XYZ_MOUSE_BUTTON_RIGHT))
		{
			return InGui::OnRightMouseButtonRelease();
		}

		return false;
	}
	bool InGuiLayer::onMouseMove(MouseMovedEvent& e)
	{
		return false;
	}
	bool InGuiLayer::onWindowResize(WindowResizeEvent& e)
	{
		InGui::OnWindowResize({ e.GetWidth(),e.GetHeight() });
		return false;
	}
	bool InGuiLayer::onKeyPressed(KeyPressedEvent& e)
	{
		InGui::OnKeyPress(e.GetKey(), e.GetMod());
		return false;
	}
	bool InGuiLayer::onMouseScroll(MouseScrollEvent& e)
	{
		return InGui::OnMouseScroll();
	}
}
