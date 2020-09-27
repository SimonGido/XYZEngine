#pragma once

#include <XYZ.h>

#include "../Tools/EditorCamera.h"

namespace XYZ {

	class SceneHierarchyPanel
	{
	public:
		SceneHierarchyPanel();
		SceneHierarchyPanel(const Ref<Scene>& context);

		void SetContext(const Ref<Scene>& context);

		void OnInGuiRender();

		void SelectEntity(const glm::vec2& position);
		
		void RemoveEntity(Entity entity);
		void InsertEntity(Entity entity);

		inline Entity GetSelectedEntity() const { return m_SelectedEntity; }
	private:
		void drawEntity(Entity entity);

	private:
		Ref<Scene> m_Context;
		Entity m_SelectedEntity;

		HashGrid2D<Entity> m_Entities;

		const uint32_t m_SceneHierarchyID = 4;
	};
}