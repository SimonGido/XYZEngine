#include "stdafx.h"
#include "SceneHierarchyPanel.h"

#include "XYZ/Core/Input.h"
#include "XYZ/Scene/SceneEntity.h"
#include "XYZ/Scene/Components.h"

#include <imgui.h>

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
        {
        }
        SceneHierarchyPanel::~SceneHierarchyPanel()
        {
        }
        void SceneHierarchyPanel::OnImGuiRender(bool& open)
        {
            if (ImGui::Begin("Scene Hierarchy", &open))
            {
                if (m_Context.Raw())
                {                  
                    drawEntityNode(SceneEntity(m_Context->m_SceneEntity, m_Context.Raw()));
                    if (ImGui::IsMouseDown(ImGuiMouseButton_Left) && ImGui::IsWindowHovered())
                        m_Context->SetSelectedEntity(Entity());
                    
                    if (ImGui::BeginPopupContextWindow(0, 1, false))
                    {
                        if (ImGui::MenuItem("Create Empty Entity"))
                            m_Context->CreateEntity("Empty Entity", GUID());
                    
                        ImGui::EndPopup();
                    }                   
                }
            }
            ImGui::End();
        }
     
        void SceneHierarchyPanel::SetContext(const Ref<Scene>& context)
        {
            m_Context = context;    
        }

        void SceneHierarchyPanel::drawEntityNode(const SceneEntity& entity)
        {
            const auto* tag = &entity.GetComponent<SceneTagComponent>().Name;
            const auto* rel = &entity.GetComponent<Relationship>();

            ImGuiTreeNodeFlags flags = (m_Context->GetSelectedEntity() == entity ? ImGuiTreeNodeFlags_Selected : 0) | ImGuiTreeNodeFlags_OpenOnArrow;
            flags |= ImGuiTreeNodeFlags_SpanAvailWidth;
            const bool opened = ImGui::TreeNodeEx((void*)(uint64_t)(uint32_t)entity, flags, tag->c_str());
            dragAndDrop(entity);
            
            if (ImGui::IsItemClicked())
            {
                m_Context->SetSelectedEntity(entity);
            }
            
            bool entityDeleted = false;
            if (ImGui::BeginPopupContextItem())
            {
                if (ImGui::MenuItem("Create Empty Entity"))
                {
                    m_Context->CreateEntity("Empty Entity", entity, GUID());
                }
                if (ImGui::MenuItem("Delete Entity"))
                {
                    entityDeleted = true;
                }
                ImGui::EndPopup();
            }
           
            // it might be invalidated
            tag = &entity.GetComponent<SceneTagComponent>().Name;
            rel = &entity.GetComponent<Relationship>();
            if (opened)
            {
                if (rel->GetFirstChild())
                    drawEntityNode(SceneEntity(rel->GetFirstChild(), m_Context.Raw()));
                ImGui::TreePop();
            }

            // it might be invalidated
            tag = &entity.GetComponent<SceneTagComponent>().Name;
            rel = &entity.GetComponent<Relationship>();
            if (rel->GetNextSibling())
                drawEntityNode(SceneEntity(rel->GetNextSibling(), m_Context.Raw()));

            if (entityDeleted)
            {
                m_Context->DestroyEntity(entity);
            }
        }
        void SceneHierarchyPanel::dragAndDrop(const SceneEntity& entity)
        {
            if (ImGui::BeginDragDropSource())
            {
                Entity id = (Entity)entity;
                ImGui::SetDragDropPayload("ENTITY_DRAG", (void*)&id, sizeof(Entity), ImGuiCond_Always);
                ImGui::EndDragDropSource();
            }
            if (ImGui::BeginDragDropTarget())
            {
                if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("ENTITY_DRAG", 0))
                {
                    const Entity child = *(Entity*)payload->Data;
                    if (child != (Entity)entity)
                    {
                        const auto& parentTransform = m_Context->m_ECS.GetComponent<TransformComponent>(entity);
                        auto& transform = m_Context->m_ECS.GetComponent<TransformComponent>(child);
                      
                        transform.DecomposeTransform(transform.WorldTransform);      
                        Relationship::SetupRelation(entity, child, m_Context->m_ECS);
                        transform.DecomposeTransform(glm::inverse(parentTransform.WorldTransform) * transform.GetTransform());
                    }
                }
                ImGui::EndDragDropTarget();
            }
        }
    }
}