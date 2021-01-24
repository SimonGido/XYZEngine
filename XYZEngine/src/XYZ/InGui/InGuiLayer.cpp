#include "stdafx.h"
#include "InGuiLayer.h"

#include "XYZ/Event/ApplicationEvent.h"
#include "XYZ/Core/Input.h"
#include "InGui.h"	

#include <glm/gtx/transform.hpp>

namespace XYZ {

	void InGuiLayer::OnAttach()
	{
		InGui::Init();

		float w = (float)Input::GetWindowSize().first;
		float h = (float)Input::GetWindowSize().second;

		m_Camera.SetProjectionMatrix(glm::ortho(0.0f, w, h, 0.0f));
	}

	void InGuiLayer::OnDetach()
	{
		InGui::Destroy();
	}

	void InGuiLayer::OnEvent(Event& event)
	{
		if (event.GetEventType() == EventType::WindowResized)
		{
			const auto& e = (WindowResizeEvent&)event;
			float w = (float)e.GetWidth();
			float h = (float)e.GetHeight();

			m_Camera.SetProjectionMatrix(glm::ortho(0.0f, w, h, 0.0f));
		}
		InGui::OnEvent(event);
	}

	void InGuiLayer::Begin()
	{
		InGui::BeginFrame(m_Camera.GetProjectionMatrix());
	}

	void InGuiLayer::End()
	{
		InGui::EndFrame();
	}

}