#include "stdafx.h"
#include "IGLayer.h"

#include "XYZ/Event/ApplicationEvent.h"
#include "XYZ/Core/Input.h"

#include "IG.h"	

#include <glm/gtx/transform.hpp>

namespace XYZ {


	void IGLayer::OnAttach()
	{
		float w = (float)Input::GetWindowSize().first;
		float h = (float)Input::GetWindowSize().second;

		m_Camera.SetProjectionMatrix(glm::ortho(0.0f, w, h, 0.0f));

		IG::Init();
	}

	void IGLayer::OnDetach()
	{
		IG::Shutdown();
	}

	void IGLayer::OnEvent(Event& event)
	{
		if (event.GetEventType() == EventType::WindowResized)
		{
			const auto& e = (WindowResizeEvent&)event;
			float w = (float)e.GetWidth();
			float h = (float)e.GetHeight();

			m_Camera.SetProjectionMatrix(glm::ortho(0.0f, w, h, 0.0f));
		}
		IG::OnEvent(event);
	}

	void IGLayer::Begin()
	{
		IG::BeginFrame(m_Camera.GetProjectionMatrix());
	}

	void IGLayer::End()
	{
		IG::EndFrame();
	}
}