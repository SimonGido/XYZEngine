#include "SceneHierarchyPanel.h"


namespace XYZ {
	SceneHierarchyPanel::SceneHierarchyPanel()
		: m_Dockspace(nullptr), m_GuiContext(nullptr)
	{
	}
	SceneHierarchyPanel::SceneHierarchyPanel(Dockspace* dockSpace, GuiContext* gui, Entity panelEntity)
		: m_Dockspace(dockSpace), m_GuiContext(gui), m_PanelEntity(panelEntity)
	{	
	}
	SceneHierarchyPanel::~SceneHierarchyPanel()
	{

	}
	void SceneHierarchyPanel::SetContext(const Ref<Scene>& scene)
	{
		for (auto ent : m_Entities)
			m_GuiContext->DestroyEntity(ent);
		m_Context = scene;

		for (auto id : m_Context->m_Entities)
		{
			Entity entity(id, &m_Context->m_ECS);
			Entity textEntity = m_GuiContext->CreateText(m_PanelEntity, TextSpecification(
				TextAlignment::Center,
				entity.GetComponent<SceneTagComponent>().Name,
				glm::vec3(0.0f),
				glm::vec2(m_PanelEntity.GetComponent<RectTransform>().Size.x, 20.0f),
				glm::vec4(1.0f)
			));
			m_Entities.push_back(textEntity);
		}
	}
	void SceneHierarchyPanel::Clean()
	{
		for (auto ent : m_Entities)
			m_GuiContext->DestroyEntity(ent);
	}
}