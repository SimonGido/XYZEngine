#pragma once

#include "XYZ/Scene/Scene.h"
#include "XYZ/IG/IG.h"

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
		IGWindow* m_Window;

		size_t m_HandleCount;
		size_t m_PoolHandle;
	};
}