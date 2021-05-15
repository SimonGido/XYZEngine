#include "stdafx.h"
#include "MainPanel.h"

#include "XYZ/Core/Input.h"
#include "XYZ/BasicUI/BasicUILoader.h"
#include "XYZ/Script/ScriptEngine.h"

namespace XYZ {
	namespace Editor {
		MainPanel::MainPanel(const std::string& filepath)
			:
			EditorUI(filepath)
		{
			SetupUI();
		}
		MainPanel::~MainPanel()
		{
		}
		void MainPanel::OnReload()
		{
			// Force to call setup UI, after everything is loaded
			m_Reloaded = true;
		}
		void MainPanel::SetupUI()
		{
			bUIScrollbox& scrollbox = bUI::GetUI<bUIScrollbox>(GetName(), "Scrollbox");
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

			bUI::GetUI<bUIButton>("Main", "Reload").Callbacks.push_back(
				[](bUICallbackType type, bUIElement& element) {
				if (type == bUICallbackType::Active)
				{
					bUI::Reload();
				}
			});

			bUI::GetUI<bUIButton>("Main", "Reload Scripts").Callbacks.push_back(
				[](bUICallbackType type, bUIElement& element) {
				if (type == bUICallbackType::Active)
				{
					ScriptEngine::ReloadAssembly("");
				}
			});

			bUIAllocator& allocator = bUI::GetAllocator(GetName());
			// Inspector
			bUICheckbox* inspectorVisible = allocator.GetElement<bUICheckbox>("Inspector");
			inspectorVisible->Checked = true;
			inspectorVisible->Callbacks.push_back(
				[&](bUICallbackType type, bUIElement& element) {
				if (type == bUICallbackType::StateChange)
				{
					bUICheckbox& casted = static_cast<bUICheckbox&>(element);
					inspectorVisibility(casted.Checked);
				}
			});
			// Inspector

			// Scene Hierarchy
			bUICheckbox* hierarchyVisible = allocator.GetElement<bUICheckbox>("Scene Hierarchy");
			hierarchyVisible->Checked = true;
			hierarchyVisible->Callbacks.push_back(
				[&](bUICallbackType type, bUIElement& element) {
				if (type == bUICallbackType::StateChange)
				{
					bUICheckbox& casted = static_cast<bUICheckbox&>(element);
					sceneHierarchyVisibility(casted.Checked);
				}
			});
			// Scene Hierarchy

			// Skinning Editor
			bUICheckbox* skinningVisible = allocator.GetElement<bUICheckbox>("Skinning Editor");
			skinningVisible->Checked = false;
			skinningEditorVisibility(false);
			skinningVisible->Callbacks.push_back(
				[&](bUICallbackType type, bUIElement& element) {
				if (type == bUICallbackType::StateChange)
				{
					bUICheckbox& casted = static_cast<bUICheckbox&>(element);
					skinningEditorVisibility(casted.Checked);
				}
			});

			// Animation Editor
			bUICheckbox* animationVisible = allocator.GetElement<bUICheckbox>("Animation Editor");
			animationVisible->Checked = false;
			animationEditorVisibility(false);
			animationVisible->Callbacks.push_back(
				[&](bUICallbackType type, bUIElement& element) {
				if (type == bUICallbackType::StateChange)
				{
					bUICheckbox& casted = static_cast<bUICheckbox&>(element);
					animationEditorVisibility(casted.Checked);
				}
			});
		}
		void MainPanel::OnUpdate(Timestep ts)
		{
			bUIAllocator& allocator = bUI::GetAllocator(GetName());
			bUIWindow* viewWindow = allocator.GetElement<bUIWindow>("View");
			bUI::SetupLayout(allocator, *viewWindow, m_ViewLayout);
			if (m_Reloaded)
			{
				m_Reloaded = false;
				SetupUI();
			}
		}
		void MainPanel::OnEvent(Event& event)
		{
			if (event.GetEventType() == EventType::WindowResized)
			{
				WindowResizeEvent& e = (WindowResizeEvent&)event;
				bUIScrollbox& scrollbox = bUI::GetUI<bUIScrollbox>(GetName(), "Scrollbox");
				scrollbox.Size.x = e.GetWidth();

				float winWidth = findPerWindowWidth();
				uint32_t counter = 0;
				bUI::ForEach<bUIWindow>(GetName(), "Scrollbox", [&](bUIWindow& window) {
					window.Size.x = winWidth;
					window.Coords.x = winWidth * counter;
					counter++;
				});
			}
		}

		float MainPanel::findPerWindowWidth()
		{
			bUIScrollbox& scrollbox = bUI::GetUI<bUIScrollbox>(GetName(), "Scrollbox");
			uint32_t count = 0;
			bUI::ForEach<bUIWindow>(GetName(), "Scrollbox", [&](bUIWindow& window) {
				count++;
			});
			return scrollbox.Size.x / (float)count;
		}
		void MainPanel::animationEditorVisibility(bool visible)
		{
			bUI::GetUI<bUIWindow>("AnimationEditor", "Animation Editor").Visible = visible;
		}
		void MainPanel::skinningEditorVisibility(bool visible)
		{
			bUI::GetUI<bUIWindow>("SkinningEditor", "Skinning Editor").Visible	= visible;
			bUI::GetUI<bUIWindow>("SkinningEditor", "Skinning Preview").Visible = visible;
			bUI::GetUI<bUIWindow>("SkinningEditor", "Bone Hierarchy").Visible	= visible;
			bUI::GetUI<bUIWindow>("SkinningEditor", "Mesh").Visible				= visible;
		}
		void MainPanel::sceneHierarchyVisibility(bool visible)
		{
			bUI::GetUI<bUIWindow>("SceneHierarchy", "Scene Hierarchy").Visible = visible;
		}
		void MainPanel::inspectorVisibility(bool visible)
		{
			bUI::GetUI<bUIWindow>("Inspector", "Inspector").Visible = visible;
		}
	}
}