#pragma once

#include <XYZ.h>

namespace XYZ {

	class SceneHierarchyPanel
	{
	public:
		SceneHierarchyPanel() = default;
		SceneHierarchyPanel(const Ref<Scene>& context);

		void SetContext(const Ref<Scene>& context);

		void OnInGuiRender();

		Entity GetSelectedEntity() const { return m_SelectedEntity; }
	private:
		void drawEntity(Entity entity);
	
	private:
		Ref<Scene> m_Context;
		Entity m_SelectedEntity;
	};
}