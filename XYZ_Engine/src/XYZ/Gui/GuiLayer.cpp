#include "stdafx.h"
#include "GuiLayer.h"

#include "XYZ/Core/Application.h"
#include "XYZ/Core/Input.h"

#include "XYZ/Renderer/Camera.h"

namespace XYZ {

	static glm::vec2 ScreenToWorld(const glm::vec2& point,const glm::vec2& windowSize, const Camera& camera)
	{
		//auto& bounds = camera.GetBounds();
		//glm::vec2 pos = { camera.GetPosition().x,camera.GetPosition().y };
		//pos.x += (point.x / windowSize.x) * bounds.GetWidth() - bounds.GetWidth() * 0.5f;
		//pos.y += bounds.GetHeight() * 0.5f - (point.y / windowSize.y) * bounds.GetHeight();
		
		//return pos;

		return { 1,1 };
	}

	GuiLayer::GuiLayer()
		//:
		//m_Camera(-((float)Application::Get().GetWindow().GetWidth() / 2), 
		//	       (float)Application::Get().GetWindow().GetWidth() / 2, 
		//	     -((float)Application::Get().GetWindow().GetHeight() / 2), 
		//	       (float)Application::Get().GetWindow().GetHeight() / 2)
	{
	}
	void GuiLayer::OnAttach()
	{
		m_GuiSystem = ECSManager::RegisterSystem<GuiSystem>();
	}
	void GuiLayer::OnDetach()
	{
	}
	void GuiLayer::OnUpdate(float dt)
	{
		m_GuiSystem->Update(dt);
	}
	void GuiLayer::OnEvent(Event& event)
	{
		EventDispatcher dispatcher(event);
		if (dispatcher.Dispatch<MouseButtonPressEvent>(Hook(&GuiLayer::onMouseButtonPress, this)))
		{ }
		else if (dispatcher.Dispatch<MouseButtonReleaseEvent>(Hook(&GuiLayer::onMouseButtonRelease, this)))
		{ }
		else if (dispatcher.Dispatch<MouseMovedEvent>(Hook(&GuiLayer::onMouseMove, this)))
		{ }
		else if (dispatcher.Dispatch<MouseScrollEvent>(Hook(&GuiLayer::onMouseScroll, this)))
		{ }
		else if (dispatcher.Dispatch<WindowResizeEvent>(Hook(&GuiLayer::onWindowResized, this)))
		{ }
	}
	bool GuiLayer::onMouseButtonPress(MouseButtonPressEvent& event)
	{
		glm::vec2 mousePos = { Input::GetMouseX(),Input::GetMouseY() };
		glm::vec2 winSize = { Input::GetWindowSize().first, Input::GetWindowSize().second };

		
		//return m_GuiSystem->OnMouseButtonPress(ScreenToWorld(mousePos, winSize, *m_Camera));
		return false;
	}
	bool GuiLayer::onMouseButtonRelease(MouseButtonReleaseEvent& event)
	{
		glm::vec2 mousePos = { Input::GetMouseX(),Input::GetMouseY() };
		glm::vec2 winSize = { Input::GetWindowSize().first, Input::GetWindowSize().second };

		//return m_GuiSystem->OnMouseButtonRelease(ScreenToWorld(mousePos, winSize, *m_Camera));
		return false;
	}
	bool GuiLayer::onMouseMove(MouseMovedEvent& event)
	{
		glm::vec2 mousePos = { Input::GetMouseX(),Input::GetMouseY() };
		glm::vec2 winSize = { Input::GetWindowSize().first, Input::GetWindowSize().second };

		//return m_GuiSystem->OnMouseMove(ScreenToWorld(mousePos, winSize, *m_Camera));
		return false;
	}
	bool GuiLayer::onMouseScroll(MouseScrollEvent& event)
	{
		glm::vec2 mousePos = { Input::GetMouseX(),Input::GetMouseY() };
		glm::vec2 winSize = { Input::GetWindowSize().first, Input::GetWindowSize().second };

		//return m_GuiSystem->OnMouseScroll(ScreenToWorld(mousePos, winSize, *m_Camera));
		return false;
	}
	bool GuiLayer::onWindowResized(WindowResizeEvent& event)
	{
		//m_Camera.SetProjection(0, event.GetWidth(), 0, event.GetHeight());
		return false;
	}
}