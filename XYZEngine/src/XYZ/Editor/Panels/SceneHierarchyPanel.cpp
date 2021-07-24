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
        void SceneHierarchyPanel::OnImGuiRender()
        {
            if (ImGui::Begin("Scene Hierarchy"))
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

        void SceneHierarchyPanel::drawEntityNode(SceneEntity entity)
        {
            auto& tag = entity.GetComponent<SceneTagComponent>().Name;
            auto& rel = entity.GetComponent<Relationship>();

            ImGuiTreeNodeFlags flags = ((m_Context->GetSelectedEntity() == entity) ? ImGuiTreeNodeFlags_Selected : 0) | ImGuiTreeNodeFlags_OpenOnArrow;
            flags |= ImGuiTreeNodeFlags_SpanAvailWidth;
            bool opened = ImGui::TreeNodeEx((void*)(uint64_t)(uint32_t)entity, flags, tag.c_str());

            if (ImGui::IsItemClicked())
            {
                m_Context->SetSelectedEntity(entity);
                if (m_Callback)
                    m_Callback(m_Context->GetSelectedEntity());
            }
            
            bool entityDeleted = false;
            if (ImGui::BeginPopupContextItem())
            {
                if (ImGui::MenuItem("Delete Entity"))
                    entityDeleted = true;

                ImGui::EndPopup();
            }
            if (opened)
            {
                if (rel.GetFirstChild())
                    drawEntityNode(SceneEntity(rel.GetFirstChild(), m_Context.Raw()));
                ImGui::TreePop();
            }
            if (rel.GetNextSibling())
                drawEntityNode(SceneEntity(rel.GetNextSibling(), m_Context.Raw()));

            if (entityDeleted)
            {
                m_Context->DestroyEntity(entity);
            }
        }
    }
}