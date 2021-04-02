#include "stdafx.h"
#include "SceneHierarchyPanel.h"

#include "XYZ/Core/Input.h"
#include "XYZ/InGui/InGui.h"
#include "XYZ/Scene/SceneEntity.h"

namespace XYZ {
    SceneHierarchyPanel::SceneHierarchyPanel()
    {
        std::initializer_list<IGHierarchyElement> types{
            {
                IGElementType::Window,
                {
                    {IGElementType::Tree,{}}
                }
            }
        };
               
        auto [poolHandle, count] =  IG::AllocateUI(types);
        m_PoolHandle = poolHandle;
        m_HandleCount = count;
        IGTree& tree = IG::GetUI<IGTree>(m_PoolHandle, 1);
        tree.OnSelect = [&](uint32_t key) {
            if (m_Context.Raw())
                m_Context->SetSelectedEntity(key);
        };
        m_Window = &IG::GetUI<IGWindow>(m_PoolHandle, 0);
        m_Window->Label = "Hierarchy Panel";
    }
    SceneHierarchyPanel::~SceneHierarchyPanel()
    {
        m_Context->m_ECS.RemoveListener<SceneTagComponent>(this);
    }
    void SceneHierarchyPanel::SetContext(Ref<Scene> context)
    {
        if (m_Context.Raw())
            m_Context->m_ECS.RemoveListener<SceneTagComponent>(this);
       

        m_Context = context;
        m_Context->m_ECS.AddListener<SceneTagComponent>([this](uint32_t entity, CallbackType type) {
            
            ECSManager& ecs = m_Context->m_ECS;
            IGTree& tree = IG::GetUI<IGTree>(m_PoolHandle, 1);
            if (type == CallbackType::ComponentCreate)
            {
                SceneTagComponent& sceneTag = ecs.GetComponent<SceneTagComponent>(entity);
                tree.AddItem(entity, IGTreeItem(sceneTag.Name));
            }
            else if (type == CallbackType::ComponentRemove || type == CallbackType::EntityDestroy)
            {
                tree.RemoveItem(entity);
            }
  
        }, this);
     
        rebuildTree();
    }
    void SceneHierarchyPanel::OnUpdate()
    {
        if (m_Context.Raw())
        {
            IGTree& tree = IG::GetUI<IGTree>(m_PoolHandle, 1);
            if (m_Context->m_SelectedEntity)
                tree.GetItem(m_Context->m_SelectedEntity).Color = IG::GetContext().RenderData.Colors[IGRenderData::HooverColor];
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
        IGTree& tree = IG::GetUI<IGTree>(m_PoolHandle, 1);
        tree.Clear();
        ECSManager& ecs = m_Context->m_ECS;
        for (uint32_t entity : m_Context->m_Entities)
        {
            SceneTagComponent& sceneTag = ecs.GetComponent<SceneTagComponent>(entity);
            tree.AddItem(entity, IGTreeItem(sceneTag.Name));
        }
    }

    void SceneHierarchyPanel::updateTree()
    {
    }

    bool SceneHierarchyPanel::onMouseButtonPress(MouseButtonPressEvent& event)
    {
        if (event.IsButtonPressed(MouseCode::MOUSE_BUTTON_RIGHT))
        {
            IGTree& tree = IG::GetUI<IGTree>(m_PoolHandle, 1);
            
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