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
		m_ViewLayout = { 10.0f, 5.0f, 10.0f, 10.0f, 10.0f, {1}, true };
		
		bUIAllocator& allocator = bUI::GetAllocator("Main");
		
		// Inspector
		bUICheckbox* inspectorVisible = allocator.GetElement<bUICheckbox>("Inspector");
		inspectorVisible->Checked = true;
		inspectorVisible->Callbacks.push_back(
			[&](bUICallbackType type, bUIElement& element) {
				if (type == bUICallbackType::StateChange)
				{
					bUICheckbox& casted = static_cast<bUICheckbox&>(element);
					bUI::GetUI<bUIWindow>("Inspector", "Inspector").Visible = casted.Checked;
				}
			}
		);
		// Inspector

		// Scene Hierarchy
		bUICheckbox* hierarchyVisible = allocator.GetElement<bUICheckbox>("Scene Hierarchy");
		hierarchyVisible->Checked = true;
		hierarchyVisible->Callbacks.push_back(
			[&](bUICallbackType type, bUIElement& element) {
				if (type == bUICallbackType::StateChange)
				{
					bUICheckbox& casted = static_cast<bUICheckbox&>(element);
					bUI::GetUI<bUIWindow>("SceneHierarchy", "Scene Hierarchy").Visible = casted.Checked;
				}
			}
		);
		// Scene Hierarchy
	}
	MainPanel::~MainPanel()
	{
		bUILoader::Save("Main", "Layouts/Main.bui");
	}
	void MainPanel::OnUpdate()
	{
		bUIAllocator& allocator = bUI::GetAllocator("Main");
		bUIWindow* viewWindow = allocator.GetElement<bUIWindow>("View");
		bUI::SetupLayout(allocator, *viewWindow, m_ViewLayout);
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