#include "SceneHierarchyPanel.h"

namespace XYZ {
	SceneHierarchyPanel::SceneHierarchyPanel(const Ref<Scene>& context)
		:
		m_Context(context)
	{
	}
	void SceneHierarchyPanel::SetContext(const Ref<Scene>& context)
	{
		m_Context = context;
	}
	void SceneHierarchyPanel::OnInGuiRender()
	{
		if (InGui::Begin("Scene Hierarchy", { 0,0 }, { 400,300 }))
		{
			if (m_Context)
			{
				for (auto ent : m_Context->m_SceneGraph.GetFlatData())
				{
					Entity entity = { ent.GetData().Entity,m_Context.Raw() };
					drawEntity(entity);
				}
			}
		}
		InGui::End();
	}
	void SceneHierarchyPanel::drawEntity(Entity entity)
	{
		if (entity.HasComponent<SceneTagComponent>())
		{
			auto& tag = entity.GetComponent<SceneTagComponent>()->Name;
			glm::vec4 textColor = { 1,1,1,1 };
			if ((uint32_t)m_SelectedEntity == (uint32_t)entity)
				textColor = { 0,1,1,1 };

			if (InGui::Text(tag, { 1,1 }, textColor))
			{
				m_SelectedEntity = entity;
			}
			InGui::Separator();
		}
	}
}