#include "ScenePanel.h"

namespace XYZ {

    static bool Collide(const glm::vec3& translation, const glm::vec3& scale, const glm::vec2& mousePos)
    {
        return (
            mousePos.x > translation.x - scale.x / 2 && mousePos.x < translation.x + scale.x / 2 &&
            mousePos.y > translation.y - scale.y / 2 && mousePos.y < translation.y + scale.y / 2
            );
    }

    ScenePanel::ScenePanel(uint32_t id)
        : 
        Panel(id),
        m_Entities(50, 100)
    {
        InGui::RenderWindow(id, "Scene", 0, { 0,0 }, { 200,200 });
        InGui::End();

        InGui::GetWindow(m_PanelID)->OnResizeCallback = Hook(&ScenePanel::onInGuiWindowResize, this);
        InGui::GetWindow(m_PanelID)->Flags &= ~InGuiWindowFlag::EventBlocking;
    }
    void ScenePanel::OnInGuiRender()
    {
        if (InGui::RenderWindow(m_PanelID, "Scene", SceneRenderer::GetFinalColorBufferRendererID(), { 0,0 }, { 200,200 }))
        {
            auto& renderConfig = InGui::GetRenderConfiguration();
            if (InGui::Icon({}, glm::vec2(40.0f, 40.0f), renderConfig.SubTexture[PLAY], renderConfig.TextureID) & InGuiReturnType::Clicked)
            {
                m_Context->OnPlay();
                m_Context->SetState(SceneState::Play);
            }
            if (InGui::Icon({}, glm::vec2(40.0f, 40.0f), renderConfig.SubTexture[PAUSE], renderConfig.TextureID) & InGuiReturnType::Clicked)
            {
                m_Context->SetState(SceneState::Edit);
            }
            InGui::Selector(m_Selecting);
        }
        InGui::End();
    }
    void ScenePanel::OnUpdate(Timestep ts)
    {
        // Update
        if (InGui::GetWindow(m_PanelID)->Flags & InGuiWindowFlag::Hoovered)
        {
            m_EditorCamera.OnUpdate(ts);
            updateModifiedEntity();
        }
        else
            m_EditorCamera.Stop();
        
        // Update and render context ( Scene )
        m_Context->OnUpdate(ts);
        if (m_Context->GetState() == SceneState::Edit)
            m_Context->OnRenderEditor(m_EditorCamera);
        else if (m_Context->GetState() == SceneState::Play)
            m_Context->OnRender();
    }

    void ScenePanel::OnEvent(Event& event)
    {
        EventDispatcher dispatcher(event);
        dispatcher.Dispatch<WindowResizeEvent>(Hook(&ScenePanel::onWindowResized, this));
        
        // Events that should be called only when scene window is hoovered
        if (InGui::GetWindow(m_PanelID)->Flags & InGuiWindowFlag::Hoovered)
        {
            m_EditorCamera.OnEvent(event);
            dispatcher.Dispatch<MouseButtonPressEvent>(Hook(&ScenePanel::onMouseButtonPress, this));
            dispatcher.Dispatch<MouseButtonReleaseEvent>(Hook(&ScenePanel::onMouseButtonRelease , this));
            dispatcher.Dispatch<KeyPressedEvent>(Hook(&ScenePanel::onKeyPress, this));
            dispatcher.Dispatch<KeyReleasedEvent>(Hook(&ScenePanel::onKeyRelease, this));
        }
    }

    void ScenePanel::SetContext(const Ref<Scene>& context)
    {
        m_Context = context;
        m_Entities.Clear();
        for (auto ent : m_Context->m_Entities)
        {
            Entity entity = { ent,m_Context.Raw() };
            if (entity.HasComponent<SceneTagComponent>())
            {
                auto transform = entity.GetComponent<TransformComponent>();
                m_Entities.Insert(entity, transform->Translation, transform->Scale);
            }
        }
    }
    
    bool ScenePanel::onWindowResized(WindowResizeEvent& event)
    {
        m_Context->SetViewportSize((uint32_t)(event.GetWidth()), (uint32_t)(event.GetHeight()));
        m_EditorCamera.OnResize(InGui::GetWindow(m_PanelID)->Size);
        return false;
    }
    bool ScenePanel::onMouseButtonPress(MouseButtonPressEvent& event)
    {
        if (event.IsButtonPressed(MouseCode::XYZ_MOUSE_BUTTON_LEFT))
        {
            auto [mx, my] = Input::GetMousePosition();
            auto [width, height] = Input::GetWindowSize();
            glm::vec2 relativeMousePos = InGui::GetWorldPosition(*InGui::GetWindow(m_PanelID), m_EditorCamera.GetPosition(), m_EditorCamera.GetAspectRatio(), m_EditorCamera.GetZoomLevel());
            m_Context->SetSelectedEntity(selectEntity(relativeMousePos));

            return true;       
        }
        else if (event.IsButtonPressed(MouseCode::XYZ_MOUSE_BUTTON_RIGHT))
        {
            
        }

        return false;
    }
    bool ScenePanel::onMouseButtonRelease(MouseButtonReleaseEvent& event)
    {
        return false;
    }
    bool ScenePanel::onKeyPress(KeyPressedEvent& event)
    {
        Entity entity = m_Context->GetSelectedEntity();
        if (entity)
        {
            if (event.IsKeyPressed(KeyCode::XYZ_KEY_DELETE))
            {
                auto transform = entity.GetComponent<TransformComponent>();
                m_Entities.Remove(entity, transform->Translation, transform->Scale);
                m_Context->DestroyEntity(entity);
                m_Context->SetSelectedEntity(Entity());
                m_ModifiedEntity.Entity = Entity();
                return true;
            }
            if (event.IsKeyPressed(KeyCode::XYZ_KEY_S))
            {
                m_ModifiedEntity.StartMousePosition = InGui::GetWorldPosition(*InGui::GetWindow(PanelID::Scene), m_EditorCamera.GetPosition(), m_EditorCamera.GetAspectRatio(), m_EditorCamera.GetZoomLevel());
                m_ModifiedEntity.Scaling = true;
                m_ModifiedEntity.Transform = entity.GetComponent<TransformComponent>();
                m_ModifiedEntity.OldTransform = *m_ModifiedEntity.Transform;
                m_ModifiedEntity.Entity = entity;
                m_Entities.Remove(entity, m_ModifiedEntity.Transform->Translation, m_ModifiedEntity.Transform->Scale);
            }
            else if (event.IsKeyPressed(KeyCode::XYZ_KEY_G))
            {
                m_ModifiedEntity.StartMousePosition = InGui::GetWorldPosition(*InGui::GetWindow(PanelID::Scene), m_EditorCamera.GetPosition(), m_EditorCamera.GetAspectRatio(), m_EditorCamera.GetZoomLevel());
                m_ModifiedEntity.Moving = true;
                m_ModifiedEntity.Transform = entity.GetComponent<TransformComponent>();
                m_ModifiedEntity.OldTransform = *m_ModifiedEntity.Transform;
                m_ModifiedEntity.Entity = entity;
                m_Entities.Remove(entity, m_ModifiedEntity.Transform->Translation, m_ModifiedEntity.Transform->Scale);
            }
            else if (event.IsKeyPressed(KeyCode::XYZ_KEY_R))
            {
                m_ModifiedEntity.StartMousePosition = InGui::GetWorldPosition(*InGui::GetWindow(PanelID::Scene), m_EditorCamera.GetPosition(), m_EditorCamera.GetAspectRatio(), m_EditorCamera.GetZoomLevel());
                m_ModifiedEntity.Rotating = true;
                m_ModifiedEntity.Transform = entity.GetComponent<TransformComponent>();
                m_ModifiedEntity.OldTransform = *m_ModifiedEntity.Transform;
                m_ModifiedEntity.Entity = entity;
            }
        }
        return false;
    }
    bool ScenePanel::onKeyRelease(KeyReleasedEvent& event)
    {
        if (event.IsKeyReleased(KeyCode::XYZ_KEY_S))
        {
            m_ModifiedEntity.Scaling = false;
            if (m_ModifiedEntity.Entity)
                m_Entities.Insert(m_ModifiedEntity.Entity, m_ModifiedEntity.Transform->Translation, m_ModifiedEntity.Transform->Scale);
            
            m_ModifiedEntity.Entity = Entity();
        }
        else if (event.IsKeyReleased(KeyCode::XYZ_KEY_G))
        {
            m_ModifiedEntity.Moving = false;
            if (m_ModifiedEntity.Entity)
                m_Entities.Insert(m_ModifiedEntity.Entity, m_ModifiedEntity.Transform->Translation, m_ModifiedEntity.Transform->Scale);
            
            m_ModifiedEntity.Entity = Entity();
        }
        else if (event.IsKeyReleased(KeyCode::XYZ_KEY_R))
        {
            m_ModifiedEntity.Rotating = false;
            m_ModifiedEntity.Entity = Entity();
        }
        return false;
    }
    void ScenePanel::onInGuiWindowResize(const glm::vec2& size)
    {
        m_EditorCamera.OnResize(size);
    }
    Entity ScenePanel::selectEntity(const glm::vec2& position)
    {
        Entity* buffer = nullptr;
        size_t count = m_Entities.GetElements(&buffer, position, { 50,50 });
        if (buffer)
        {
            for (size_t i = 0; i < count; ++i)
            {
                auto transform = buffer[i].GetComponent<TransformComponent>();
                if (Collide(transform->Translation, transform->Scale, position))
                {
                    InGui::GetWindow(PanelID::SceneHierarchy)->Flags |= InGuiWindowFlag::Modified;
                    Entity entity = buffer[i];
                    delete[] buffer;
                    return entity;
                }
            }
            delete[]buffer;
        }
        return Entity();
    }
    void ScenePanel::updateModifiedEntity()
    {
        if (m_ModifiedEntity.Entity)
        {
            if (Input::IsKeyPressed(KeyCode::XYZ_KEY_S))
            {
                Entity entity = { m_Context->GetSelectedEntity(), m_Context.Raw() };
                auto mousePos = InGui::GetWorldPosition(*InGui::GetWindow(PanelID::Scene), m_EditorCamera.GetPosition(), m_EditorCamera.GetAspectRatio(), m_EditorCamera.GetZoomLevel());
                glm::vec3 scale = m_ModifiedEntity.OldTransform.Scale;
                scale.x += mousePos.x - m_ModifiedEntity.StartMousePosition.x;
                scale.y += mousePos.y - m_ModifiedEntity.StartMousePosition.y;
                m_ModifiedEntity.Transform->Scale = scale;
            }
            else if (Input::IsKeyPressed(KeyCode::XYZ_KEY_G))
            {
                auto mousePos = InGui::GetWorldPosition(*InGui::GetWindow(PanelID::Scene), m_EditorCamera.GetPosition(), m_EditorCamera.GetAspectRatio(), m_EditorCamera.GetZoomLevel());
                glm::vec3 translation = m_ModifiedEntity.OldTransform.Translation;
                translation.x += (mousePos.x - m_ModifiedEntity.StartMousePosition.x);
                translation.y += (mousePos.y - m_ModifiedEntity.StartMousePosition.y);
                m_ModifiedEntity.Transform->Translation = translation;
            }
            else if (Input::IsKeyPressed(KeyCode::XYZ_KEY_R))
            {
                auto mousePos = InGui::GetWorldPosition(*InGui::GetWindow(PanelID::Scene), m_EditorCamera.GetPosition(), m_EditorCamera.GetAspectRatio(), m_EditorCamera.GetZoomLevel());
                float rotation = m_ModifiedEntity.OldTransform.Rotation.x + (mousePos.x - m_ModifiedEntity.StartMousePosition.x);
                m_ModifiedEntity.Transform->Rotation.z = rotation;
            }
        }
    }
}