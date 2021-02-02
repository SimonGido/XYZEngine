#include "SceneHierarchyPanel.h"


namespace XYZ {
    SceneHierarchyPanel::SceneHierarchyPanel(uint32_t panelID)
        : 
        m_PanelID(panelID)
    {
        InGui::Begin(panelID, "Scene Hierarchy", glm::vec2(0.0f), glm::vec2(200.0f));
        InGui::End();

        uint16_t flags = InGui::GetWindow(panelID).Flags;
        flags &= ~InGuiWindowFlags::EventBlocking;
        InGui::SetWindowFlags(panelID, flags);
    }
    SceneHierarchyPanel::~SceneHierarchyPanel()
    {
        if (m_EntitiesOpen)
            delete m_EntitiesOpen;
    }
    void SceneHierarchyPanel::SetContext(Ref<Scene> context)
    {
        m_Context = context;
        m_SelectedEntityIndex = m_Context->m_Entities.size();
        if (m_CurrentSize < context->m_Entities.size())
            resizeEntities();
    }
    void SceneHierarchyPanel::OnInGuiRender()
    {
        if (InGui::Begin(m_PanelID, "Scene Hierarchy", glm::vec2(0.0f), glm::vec2(200.0f)))
        {
            if (m_Context.Raw())
            {
                if (m_CurrentSize < m_Context->m_Entities.size())
                    resizeEntities();
                uint32_t counter = 0;
                for (auto entityID : m_Context->m_Entities)
                {
                    SceneEntity entity(entityID, m_Context.Raw());
                    bool hightlight = m_SelectedEntityIndex == counter;
                    if (uint8_t res = InGui::PushNode(entity.GetComponent<SceneTagComponent>().Name.c_str(), glm::vec2(25.0f), m_EntitiesOpen[counter], hightlight))
                    {
                        if (IS_SET(res, InGuiReturnType::Clicked))
                        {
                            m_SelectedEntityIndex = (size_t)counter;
                            m_Context->SetSelectedEntity(entity);
                        }
                    }
                    counter++;
                }
         
                InGui::SetPosition(m_DropdownPosition);
                if (m_DropdownOpen)
                {
                    if (IS_SET(InGui::Dropdown("New Entity", { 150.0f, 30.0f }, m_DropdownOpen), InGuiReturnType::Clicked))
                    {

                    }
                }        
            }
        }
        InGui::End();
    }
    void SceneHierarchyPanel::OnEvent(Event& event)
    {
        EventDispatcher dispatcher(event);
        dispatcher.Dispatch<MouseButtonPressEvent>(Hook(&SceneHierarchyPanel::onMouseButtonPress, this));
        dispatcher.Dispatch<KeyPressedEvent>(Hook(&SceneHierarchyPanel::onKeyPressed, this));
    }
    void SceneHierarchyPanel::resizeEntities()
    {
        if (m_EntitiesOpen)
        {
            delete[] m_EntitiesOpen;
            m_EntitiesOpen = nullptr;
        }
        m_EntitiesOpen = new bool[m_Context->m_Entities.size()];
        memset(m_EntitiesOpen, 0, m_Context->m_Entities.size());
        m_CurrentSize = m_Context->m_Entities.size();
    }
    bool SceneHierarchyPanel::onMouseButtonPress(MouseButtonPressEvent& event)
    {
        if (event.IsButtonPressed(MouseCode::MOUSE_BUTTON_RIGHT))
        {
            if (IS_SET(InGui::GetWindow(m_PanelID).Flags, InGuiWindowFlags::Hoovered))
            {
                m_DropdownPosition = { Input::GetMouseX(), Input::GetMouseY() };
                m_DropdownOpen = true;
                return true;
            }
        }
        return false;
    }
    bool SceneHierarchyPanel::onKeyPressed(KeyPressedEvent& event)
    {
        if (event.IsKeyPressed(KeyCode::KEY_DELETE))
        {
            if (m_Context.Raw())
            {
                if (auto entity = m_Context->GetSelectedEntity())
                {
                    m_Context->DestroyEntity(entity);
                    m_SelectedEntityIndex = m_Context->m_Entities.size();
                }
            }
        }
        return false;
    }
}