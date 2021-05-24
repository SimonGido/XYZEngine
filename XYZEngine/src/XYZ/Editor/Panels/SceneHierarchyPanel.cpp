#include "stdafx.h"
#include "SceneHierarchyPanel.h"

#include "XYZ/Core/Input.h"
#include "XYZ/Scene/SceneEntity.h"

#include "XYZ/InGui/InGui.h"

namespace XYZ {
    namespace Helper {
        static bool Collide(const glm::vec2& pos, const glm::vec2& size, const glm::vec2& point)
        {
            return (pos.x + size.x > point.x &&
                pos.x		   < point.x&&
                pos.y + size.y >  point.y &&
                pos.y < point.y);
        }
    }
    namespace Editor {
        SceneHierarchyPanel::SceneHierarchyPanel()
            :
            m_Open(nullptr)
        {
        }
        SceneHierarchyPanel::~SceneHierarchyPanel()
        {
            m_Context->m_ECS.RemoveListener<SceneTagComponent>(this);
            if (m_Open)
                delete[]m_Open;
        }
        void SceneHierarchyPanel::OnUpdate()
        {
            if (InGui::Begin("Scene Hierarchy"), 
                  InGuiWindowStyleFlags::PanelEnabled
                | InGuiWindowStyleFlags::ScrollEnabled
                | InGuiWindowStyleFlags::LabelEnabled)
            {
                if (m_Context.Raw())
                {
                    uint32_t depth = 0;
                    ECSManager& ecs = m_Context->m_ECS;
                    std::stack<Entity> entities;
                    entities.push(m_Context->m_SceneEntity);
  
                    while (!entities.empty())
                    {
                        Entity tmp = entities.top();
                        entities.pop();

                        const Relationship& relation = ecs.GetComponent<Relationship>(tmp);
                        

                        if (relation.GetDetph() > depth)
                            InGui::BeginTreeChild();
                        while (relation.GetDetph() < depth)
                        {
                            InGui::EndTreeChild();
                            depth--;
                        }
                        depth = relation.GetDetph();
                        const SceneTagComponent& sceneTag = ecs.GetComponent<SceneTagComponent>(tmp);
                        
                        if (m_Context->GetSelectedEntity() == tmp)
                            InGui::EnableHighlight();
                        if (IS_SET(InGui::TreeNode(sceneTag.Name.c_str(), glm::vec2(25.0f), m_Open[tmp]), InGui::Pressed))
                        {
                            m_Context->SetSelectedEntity(tmp);
                        }
                        InGui::DisableHighlight();
                       
                        if (relation.GetNextSibling())
                            entities.push(relation.GetNextSibling());
                        if (m_Open[tmp])
                        {
                            if (relation.GetFirstChild())
                                entities.push(relation.GetFirstChild());
                        }
                    }
                    while (depth)
                    {
                        InGui::EndTreeChild();
                        depth--;
                    }
                }
            }
            InGui::End();
        }

       

        void SceneHierarchyPanel::SetContext(const Ref<Scene>& context)
        {
            if (m_Context.Raw())
            {
                m_Context->m_ECS.RemoveListener<SceneTagComponent>(this);
                delete[]m_Open;
            }
            m_Context = context;
            m_Open = new bool[m_Context->m_ECS.GetHighestID()];
            memset(m_Open, 0, m_Context->m_ECS.GetHighestID());      
        }

        void SceneHierarchyPanel::OnEvent(Event& event)
        {
            EventDispatcher dispatcher(event);
            dispatcher.Dispatch<MouseButtonPressEvent>(Hook(&SceneHierarchyPanel::onMouseButtonPress, this));
            dispatcher.Dispatch<KeyPressedEvent>(Hook(&SceneHierarchyPanel::onKeyPressed, this));
        }

        bool SceneHierarchyPanel::onMouseButtonPress(MouseButtonPressEvent& event)
        {
            auto [mx, my] = Input::GetMousePosition();
            glm::vec2 mousePosition(mx, my);
           
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
                    }
                }
            }
            return false;
        }
    }
}