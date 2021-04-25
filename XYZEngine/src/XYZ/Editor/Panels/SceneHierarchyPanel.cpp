#include "stdafx.h"
#include "SceneHierarchyPanel.h"

#include "XYZ/Core/Input.h"
#include "XYZ/Scene/SceneEntity.h"

#include "XYZ/BasicUI/BasicUILoader.h"

namespace XYZ {
    SceneHierarchyPanel::SceneHierarchyPanel()
    {
        bUILoader::Load("Layouts/SceneHierarchy.bui");
        m_Window = &bUI::GetUI<bUIWindow>("SceneHierarchy", "Scene Hierarchy");
        m_Tree = &bUI::GetUI<bUITree>("SceneHierarchy", "Hierarchy Tree");

        m_Tree->OnSelect = [&](uint32_t entity) {
            if (m_Context.Raw())
            {
                if (m_Context->GetSelectedEntity() == entity)
                    m_Context->SetSelectedEntity(Entity());
                else
                    m_Context->SetSelectedEntity(entity);
            } 
        };
    }
    SceneHierarchyPanel::~SceneHierarchyPanel()
    {
        m_Context->m_ECS.RemoveListener<SceneTagComponent>(this);
        bUILoader::Save("SceneHierarchy", "Layouts/SceneHierarchy.bui");
    }
    void SceneHierarchyPanel::SetContext(Ref<Scene> context)
    {
        if (m_Context.Raw())
            m_Context->m_ECS.RemoveListener<SceneTagComponent>(this);
       
        m_Context = context;
        m_Context->m_ECS.AddListener<SceneTagComponent>([this](uint32_t entity, CallbackType type) {
            
            ECSManager& ecs = m_Context->m_ECS;
            if (type == CallbackType::ComponentCreate)
            {
                SceneTagComponent& sceneTag = ecs.GetComponent<SceneTagComponent>(entity);
                m_Tree->AddItem(entity, bUITreeItem(sceneTag.Name));
            }
            else if (type == CallbackType::ComponentRemove || type == CallbackType::EntityDestroy)
            {
                m_Tree->RemoveItem(entity);
            }
  
        }, this);
     
        rebuildTree();
    }
    void SceneHierarchyPanel::OnUpdate()
    {
        if (m_Context.Raw())
        {
            if (m_Context->m_SelectedEntity)
                m_Tree->GetItem(m_Context->m_SelectedEntity).Color = glm::vec4(0.2f, 0.7f, 1.0f, 1.0f);
        }
    }
    void SceneHierarchyPanel::OnEvent(Event& event)
    {
        EventDispatcher dispatcher(event);
        dispatcher.Dispatch<MouseButtonPressEvent>(Hook(&SceneHierarchyPanel::onMouseButtonPress, this));
        dispatcher.Dispatch<KeyPressedEvent>(Hook(&SceneHierarchyPanel::onKeyPressed, this));
    }
    void SceneHierarchyPanel::rebuildTree()
    {
        m_Tree->Clear();
        ECSManager& ecs = m_Context->m_ECS;
        for (uint32_t entity : m_Context->m_Entities)
        {
            SceneTagComponent& sceneTag = ecs.GetComponent<SceneTagComponent>(entity);
            m_Tree->AddItem(entity, bUITreeItem(sceneTag.Name));
        }
    }

    void SceneHierarchyPanel::updateTree()
    {
    }

    bool SceneHierarchyPanel::onMouseButtonPress(MouseButtonPressEvent& event)
    {
        if (event.IsButtonPressed(MouseCode::MOUSE_BUTTON_RIGHT))
        {
            
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
                }
            }
        }
        return false;
    }
}