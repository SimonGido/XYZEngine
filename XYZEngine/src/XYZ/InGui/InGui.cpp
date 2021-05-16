#include "stdafx.h"
#include "InGui.h"

#include "XYZ/Core/Input.h"

namespace XYZ {
	static InGuiContext s_Context;
	
	void InGui::BeginFrame()
	{
		auto [mx, my] = Input::GetMousePosition();
		s_Context.m_FrameData.MousePosition = { mx, my };
	}

	void InGui::EndFrame()
	{
		s_Context.Render();
	}

	bool InGui::Begin(const char* name)
	{
		InGuiFrame& frame = s_Context.m_FrameData;
		InGuiConfig& config = s_Context.m_Config;

		XYZ_ASSERT(!frame.CurrentWindow, "Missing end call");
		InGuiWindow* window = s_Context.GetWindow(name);
		if (!window)
			window = s_Context.CreateWindow(name);
		

		frame.CurrentWindow = window;
		window->DrawList.Clear();
		window->DrawList.SetClipRect(window->ClipRect(), window->ID);

		glm::vec4 color = config.GetColor(InGuiConfig::DefaultColor);
		if (window->HandleHoover(frame.MousePosition))
			color = config.GetColor(InGuiConfig::HighlightColor);
		
		window->HandleResize(frame.MousePosition);
		if (IS_SET(window->Flags, InGuiWindow::Moving))
		{
			window->Position = frame.MousePosition - frame.MovedWindowOffset;
		}
		
		window->PushItselfToDrawlist(color, config);
		return !IS_SET(window->Flags, InGuiWindow::Collapsed);
	}
	void InGui::End()
	{
		XYZ_ASSERT(s_Context.m_FrameData.CurrentWindow, "Missing begin call");
		s_Context.m_FrameData.CurrentWindow = nullptr;
	}
	InGuiContext& InGui::GetContext()
	{
		return s_Context;
	}
}