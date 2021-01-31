#include "SceneHierarchyPanel.h"


namespace XYZ {
    SceneHierarchyPanel::SceneHierarchyPanel(uint32_t panelID)
        : 
        m_PanelID(panelID)
    {
        InGui::Begin(panelID, "Scene Hierarchy", glm::vec2(0.0f), glm::vec2(200.0f));
        InGui::End();
    }
    SceneHierarchyPanel::~SceneHierarchyPanel()
    {
        if (m_BranchesOpen)
            delete m_BranchesOpen;
    }
    void SceneHierarchyPanel::SetContext(Ref<Scene> context)
    {
        if (m_BranchesOpen)
        {
            delete[] m_BranchesOpen;
            m_BranchesOpen = nullptr;
        }
        m_Context = context;
        m_BranchesOpen = new bool[m_Context->m_Entities.size()];
    }
    void SceneHierarchyPanel::OnInGuiRender()
    {
        if (InGui::Begin(m_PanelID, "Scene Hierarchy", glm::vec2(0.0f), glm::vec2(200.0f)))
        {
            if (m_Context.Raw())
            {
                uint32_t counter = 0;
                for (auto entityID : m_Context->m_Entities)
                {
                    SceneEntity entity(entityID, m_Context.Raw());
                    if (InGui::PushNode(entity.GetComponent<SceneTagComponent>().Name.c_str(), glm::vec2(25.0f), m_BranchesOpen[counter]))
                    {

                    }
                    counter++;
                }
            }
        }
        InGui::End();
    }
}