#include "stdafx.h"
#include "SceneHierarchyPanel.h"

#include "XYZ/Core/Input.h"
#include "XYZ/Scene/SceneEntity.h"

#include "XYZ/BasicUI/BasicUILoader.h"

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
        SceneHierarchyPanel::SceneHierarchyPanel(const std::string& filepath)
            :
            EditorUI(filepath)
        {
            SetupUI();
        }
        SceneHierarchyPanel::~SceneHierarchyPanel()
        {
            m_Context->m_ECS.RemoveListener<SceneTagComponent>(this);
        }
        void SceneHierarchyPanel::OnUpdate(Timestep ts)
        {
            if (m_Context.Raw())
            {
                if (m_Context->m_SelectedEntity)
                    m_Tree->GetItem(m_Context->m_SelectedEntity).Color = glm::vec4(0.2f, 0.7f, 1.0f, 1.0f);
            }
        }

        void SceneHierarchyPanel::OnReload()
        {
            SetupUI();
            if (m_Context.Raw())
            {
                ECSManager& ecs = m_Context->m_ECS;
                for (Entity entity : m_Context->GetEntities())
                {
                    SceneTagComponent& sceneTag = ecs.GetComponent<SceneTagComponent>(entity);
                    m_Tree->AddItem(entity, bUIHierarchyItem(sceneTag.Name));
                }
            }
        }
        
        void SceneHierarchyPanel::SetupUI()
        {
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
            m_Image = &bUI::GetUI<bUIImage>("SceneHierarchy", "Create Entity");
            m_Image->FitParent = false;
            m_Image->Visible = false;
            m_Image->Callbacks.push_back(
                [&](bUICallbackType type, bUIElement& element) {
                if (type == bUICallbackType::Active)
                {
                    bUIImage& casted = static_cast<bUIImage&>(element);
                    if (m_Context.Raw())
                        m_Context->CreateEntity("New Entity", GUID());
                    casted.Visible = false;
                }
            });

            Ref<Texture> plusTexture = Texture2D::Create({}, "Assets/Textures/Plus.png");
            m_Image->ImageSubTexture = Ref<SubTexture>::Create(plusTexture, glm::vec2(0.0f));
        }

        void SceneHierarchyPanel::SetContext(const Ref<Scene>& context)
        {
            if (m_Context.Raw())
                m_Context->m_ECS.RemoveListener<SceneTagComponent>(this);
            m_Context = context;

            Relationship::OnParentChanged = [&](Entity entity, ECSManager& ecs) {
                Entity parent = ecs.GetComponent<Relationship>(entity).GetParent();
                m_Tree->SetParent(entity, parent);
            };
            m_Context->m_ECS.AddListener<SceneTagComponent>([this](uint32_t entity, CallbackType type) {

                ECSManager& ecs = m_Context->m_ECS;
                if (type == CallbackType::ComponentCreate)
                {
                    SceneTagComponent& sceneTag = ecs.GetComponent<SceneTagComponent>(entity);
                    Relationship& relation = ecs.GetComponent<Relationship>(entity);
                    if (relation.GetParent())
                        m_Tree->AddItem(entity, relation.GetParent(), bUIHierarchyItem(sceneTag.Name));
                    else
                        m_Tree->AddItem(entity, bUIHierarchyItem(sceneTag.Name));
                }
                else if (type == CallbackType::ComponentRemove || type == CallbackType::EntityDestroy)
                {
                    m_Tree->RemoveItem(entity);
                }

            }, this);

            rebuildTree();
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

            std::stack<Entity> entities;
            entities.push(m_Context->m_SceneEntity);
            while (!entities.empty())
            {
                Entity tmp = entities.top();
                entities.pop();

                const Relationship& relation = ecs.GetComponent<Relationship>(tmp);
                if (relation.GetNextSibling())
                    entities.push(relation.GetNextSibling());
                if (relation.GetFirstChild())
                    entities.push(relation.GetFirstChild());

                const SceneTagComponent& sceneTag = ecs.GetComponent<SceneTagComponent>(tmp);
                if (relation.GetParent())
                    m_Tree->AddItem(tmp, relation.GetParent(), bUIHierarchyItem(sceneTag.Name));
                else
                    m_Tree->AddItem(tmp, bUIHierarchyItem(sceneTag.Name));
            }
        }

        void SceneHierarchyPanel::updateTree()
        {
        }

        bool SceneHierarchyPanel::onMouseButtonPress(MouseButtonPressEvent& event)
        {
            auto [mx, my] = Input::GetMousePosition();
            glm::vec2 mousePosition(mx, my);
            if (Helper::Collide(m_Window->Coords, m_Window->Size, mousePosition))
            {
                if (event.IsButtonPressed(MouseCode::MOUSE_BUTTON_RIGHT))
                {
                    glm::vec2 position = mousePosition - m_Window->Coords - (m_Image->Size / 2.0f);
                    m_Image->Visible = !m_Image->Visible;
                    m_Image->Coords = position;
                }
                else
                {
                    m_Image->Visible = false;
                }
                return true;
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
}