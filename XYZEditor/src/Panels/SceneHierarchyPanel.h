#pragma once

#include <XYZ.h>

namespace XYZ {

	class SceneHierarchyPanel
	{
	public:
		SceneHierarchyPanel(uint32_t panelID);
		~SceneHierarchyPanel();

		void SetContext(Ref<Scene> context);


		void OnInGuiRender();

	private:
		uint32_t m_PanelID;
		Ref<Scene> m_Context;

		bool* m_BranchesOpen = nullptr;
	};
}