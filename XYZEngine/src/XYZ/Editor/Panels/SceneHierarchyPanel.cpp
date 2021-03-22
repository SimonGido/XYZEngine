#include "stdafx.h"
#include "SceneHierarchyPanel.h"

#include "XYZ/Core/Input.h"
#include "XYZ/InGui/InGui.h"
#include "XYZ/Scene/SceneEntity.h"
#include "XYZ/IG/IG.h"

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
               
        auto [id, count] =  IG::AllocateUI(types, &m_Handles);
        m_ID = id;
        m_HandleCount = count;
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
        m_Context->m_ECS.AddListener<SceneTagComponent>([](void* instance, uint32_t entity, CallbackType type) {
            

        }, this);
     
        rebuildTree();
    }
    void SceneHierarchyPanel::OnInGuiRender()
    {
        
    }
    void SceneHierarchyPanel::OnEvent(Event& event)
    {
        EventDispatcher dispatcher(event);
        dispatcher.Dispatch<MouseButtonPressEvent>(Hook(&SceneHierarchyPanel::onMouseButtonPress, this));
        dispatcher.Dispatch<KeyPressedEvent>(Hook(&SceneHierarchyPanel::onKeyPressed, this));
    }
    void SceneHierarchyPanel::rebuildTree()
    {
        IGTree& tree = IG::GetUI<IGTree>(m_ID, m_Handles[1]);
        tree.Clear();
        ECSManager& ecs = m_Context->m_ECS;
        for (uint32_t entity : m_Context->m_Entities)
        {
            IDComponent& id = ecs.GetComponent<IDComponent>(entity);
            SceneTagComponent& sceneTag = ecs.GetComponent<SceneTagComponent>(entity);
 
            std::string idStr = (std::string)id.ID;
            tree.AddItem(idStr.c_str(), nullptr, IGTreeItem(sceneTag.Name));
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