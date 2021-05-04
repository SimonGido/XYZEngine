#include "stdafx.h"
#include "MainPanel.h"

#include "XYZ/Core/Input.h"
#include "XYZ/BasicUI/BasicUILoader.h"

namespace XYZ {
	namespace Editor {
		MainPanel::MainPanel()
		{
			bUILoader::Load("Layouts/Main.bui");
			setupUI();
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
		void MainPanel::setupUI()
		{
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

			bUI::SetOnReloadCallback("Main", [&](bUIAllocator& allocator) {
				setupUI();
			});

			bUI::GetUI<bUIButton>("Main", "Reload").Callbacks.push_back(
				[](bUICallbackType type, bUIElement& element) {
				if (type == bUICallbackType::Active)
				{
					bUI::Reload();
				}
			}
			);

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

			// Skinning Editor
			bUICheckbox* skinningVisible = allocator.GetElement<bUICheckbox>("Skinning Editor");
			skinningVisible->Checked = true;
			skinningVisible->Callbacks.push_back(
				[&](bUICallbackType type, bUIElement& element) {
				if (type == bUICallbackType::StateChange)
				{
					bUICheckbox& casted = static_cast<bUICheckbox&>(element);
					bUI::GetUI<bUIWindow>("SkinningEditor", "Skinning Editor").Visible = casted.Checked;
					bUI::GetUI<bUIWindow>("SkinningEditor", "Skinning Preview").Visible = casted.Checked;
					bUI::GetUI<bUIWindow>("SkinningEditor", "Bone Hierarchy").Visible = casted.Checked;
					bUI::GetUI<bUIWindow>("SkinningEditor", "Mesh").Visible = casted.Checked;
				}
			}
			);

			// Skinning Editor
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
}