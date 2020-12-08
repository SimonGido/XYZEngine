#pragma once

#include <XYZ.h>

namespace XYZ {

	class SceneHierarchyPanel
	{
	public:
		SceneHierarchyPanel(Dockspace* dockSpace, GuiContext* gui);

		void SetContext(const Ref<Scene>& scene);



	private:
		Ref<Scene> m_Context;
		Dockspace*  m_Dockspace;
		GuiContext* m_GuiContext;
		uint32_t    m_PanelEntity;
		std::vector<uint32_t> m_Entities;
	};
}