#include "SceneHierarchyPanel.h"


namespace XYZ {
	SceneHierarchyPanel::SceneHierarchyPanel(Dockspace* dockSpace, GuiContext* gui)
		: m_GuiContext(gui)
	{
	}
	void SceneHierarchyPanel::SetContext(const Ref<Scene>& scene)
	{
		for (auto ent : m_Entities)
			m_GuiContext->DestroyEntity(ent);
		m_Context = scene;
		
	}
}