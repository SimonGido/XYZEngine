#include "stdafx.h"
#include "GuiLayer.h"

#include "XYZ/Core/Input.h"

namespace XYZ {

	GuiLayer::GuiLayer()
	{
	}
	void GuiLayer::OnAttach()
	{		
	}
	void GuiLayer::OnDetach()
	{
		for (auto context : m_GuiContexts)
			delete context;
	}
	void GuiLayer::OnUpdate(Timestep ts)
	{
		for (auto context : m_GuiContexts)
		{
			context->OnUpdate(ts);
			context->OnRender();
		}
	}
	void GuiLayer::OnEvent(Event& event)
	{
		for (auto context : m_GuiContexts)
		{
			if (event.Handled)
				break;
			context->OnEvent(event);
		}
	}
	GuiContext* GuiLayer::CreateContext(ECSManager* ecs, const GuiSpecification& specs)
	{
		auto [width, height] = Input::GetWindowSize();
		m_GuiContexts.push_back(new GuiContext(ecs, specs));
		auto last = m_GuiContexts.back();
		last->SetViewportSize(width, height);

		return m_GuiContexts.back();
	}
}