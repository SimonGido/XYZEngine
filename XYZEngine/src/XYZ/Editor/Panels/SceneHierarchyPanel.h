#pragma once

#include "XYZ/Scene/Scene.h"
#include "XYZ/BasicUI/BasicUI.h"

namespace XYZ {

	class SceneHierarchyPanel
	{
	public:
		SceneHierarchyPanel();
		~SceneHierarchyPanel();

		void SetContext(Ref<Scene> context);

		void OnUpdate();
		void OnEvent(Event& event);

	private:
		void rebuildTree();
		void updateTree();

		bool onMouseButtonPress(MouseButtonPressEvent& event);
		bool onKeyPressed(KeyPressedEvent& event);

	private:
		Ref<Scene> m_Context;
		bUIWindow* m_Window;
		bUITree* m_Tree;
		bUIImage* m_Image;
	};
}