#pragma once

#include <XYZ.h>


namespace XYZ {

	class SceneHierarchyPanel
	{
	public:
		SceneHierarchyPanel();
		SceneHierarchyPanel(const Ref<Scene>& context);

		void SetContext(const Ref<Scene>& context);

		bool OnInGuiRender();

		void SelectEntity(const glm::vec2& position);
		void InvalidateEntity();

		void RemoveEntity(Entity entity);
		void InsertEntity(Entity entity);
	
		inline Entity GetSelectedEntity() const { return m_SelectedEntity; }
	private:
		void drawEntity(Entity entity);

	private:
		Ref<Scene> m_Context;

		InGuiWindow* m_Window;
		bool m_PopupEnabled = false;
		glm::vec2 m_PopupPosition = { 0,0 };

		Entity m_SelectedEntity;
		HashGrid2D<Entity> m_Entities;
	};
}