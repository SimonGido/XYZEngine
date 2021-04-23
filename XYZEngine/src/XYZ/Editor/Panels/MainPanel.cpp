#include "stdafx.h"
#include "MainPanel.h"

#include "XYZ/Core/Input.h"
#include "XYZ/BasicUI/BasicUILoader.h"

namespace XYZ {
	MainPanel::MainPanel()
	{
		bUILoader::Load("Layouts/Main.bui");
		bUIScrollbox& scrollbox = bUI::GetUI<bUIScrollbox>("Main", "Scrollbox");
		scrollbox.EnableScroll = false;
		auto [width, height] = Input::GetWindowSize();
		scrollbox.Size.x = width;
		float winWidth = findPerWindowWidth();

		uint32_t counter = 0;
		bUI::ForEach<bUIWindow>("Main", "Scrollbox", [&](bUIWindow& window) {
			window.Size.x = winWidth;
			window.Coords.x = winWidth * counter;
			window.ChildrenVisible = false;
			counter++;
		});
	}
	void MainPanel::OnUpdate()
	{

	}
	void MainPanel::OnEvent(Event& event)
	{
		if (event.GetEventType() == EventType::WindowResized)
		{
			WindowResizeEvent& e = (WindowResizeEvent&)event;
			bUIScrollbox& scrollbox = bUI::GetUI<bUIScrollbox>("Main", "Scrollbox");
			scrollbox.Size.x = e.GetWidth();

			float winWidth = findPerWindowWidth();
			uint32_t counter = 0;
			bUI::ForEach<bUIWindow>("Main", "Scrollbox", [&](bUIWindow& window) {
				window.Size.x = winWidth;
				window.Coords.x = winWidth * counter;
				counter++;
			});
		}
	}
	float MainPanel::findPerWindowWidth()
	{
		bUIScrollbox& scrollbox = bUI::GetUI<bUIScrollbox>("Main", "Scrollbox");
		uint32_t count = 0;
		bUI::ForEach<bUIWindow>("Main", "Scrollbox", [&](bUIWindow& window) {
			count++;
		});
		return scrollbox.Size.x / (float)count;
	}
}