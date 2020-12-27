#pragma once

#include <XYZ.h>

namespace XYZ {

	class SceneHierarchyPanel
	{
	public:
		SceneHierarchyPanel();
		SceneHierarchyPanel(Dockspace* dockSpace, GuiContext* gui, Entity panelEntity);
		~SceneHierarchyPanel();

		void SetContext(const Ref<Scene>& scene);
		void Clean();

	private:
		Ref<Scene>  m_Context;
		Dockspace*  m_Dockspace;
		GuiContext* m_GuiContext;
		Entity      m_PanelEntity;
		std::vector<uint32_t> m_Entities;
	};
}