#pragma once

#include "XYZ/Scene/Scene.h"

namespace XYZ {

	class SceneHierarchyPanel
	{
	public:
		SceneHierarchyPanel();
		~SceneHierarchyPanel();

		void SetContext(Ref<Scene> context);

		void OnInGuiRender();
		void OnEvent(Event& event);
		void OnSceneTagChanged(uint32_t entity, const SceneTagComponent& oldTag, const SceneTagComponent& newTag);

	private:
		void rebuildTree();
		void updateTree();

		bool onMouseButtonPress(MouseButtonPressEvent& event);
		bool onKeyPressed(KeyPressedEvent& event);

	private:
		Ref<Scene> m_Context;

		size_t* m_Handles;
		size_t m_HandleCount;
		size_t m_ID;
	};
}