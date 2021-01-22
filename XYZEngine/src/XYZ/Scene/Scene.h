#pragma once
#include "XYZ/Core/Ref.h"
#include "XYZ/Core/Timestep.h"
#include "XYZ/ECS/ECSManager.h"

#include "XYZ/Event/Event.h"
#include "XYZ/Renderer/Camera.h"

#include "XYZ/Editor/EditorCamera.h"
#include "SceneCamera.h"
#include "Components.h"

#include "Serializable.h"

namespace XYZ {

   
    enum class SceneState
    {
        Play,
        Edit,
        Pause
    };

    /*! @class Scene
    *	@brief Holds all data relevant to a Scene
    */
    template <typename T>
    class Asset;
    class SceneEntity;
    class Scene : public RefCount,
                  public Serializable
    {
    public:
        Scene(const std::string& name);
        ~Scene();
    
        SceneEntity CreateEntity(const std::string& name, const GUID& guid);
        void DestroyEntity(SceneEntity entity);
        void SetState(SceneState state) { m_State = state; }
        void SetViewportSize(uint32_t width, uint32_t height);
        void SetSelectedEntity(uint32_t entity) { m_SelectedEntity = entity; }

        void OnAttach();
        void OnPlay();
        void OnDetach();
        void OnEvent(Event& e);
        void OnUpdate(Timestep ts);
        void OnRender();
        void OnRenderEditor(const EditorCamera& camera);

        SceneEntity GetEntity(uint32_t index);
        SceneEntity GetSelectedEntity();
        
        SceneState GetState() const { return m_State; }
        ECSManager& GetECS() { return m_ECS; }
        const GUID& GetUUID() const { return m_UUID; }

        inline const std::string& GetName() const { return m_Name; }
    private:
        void showSelection(uint32_t entity);
        void showCamera(uint32_t entity);

        template <typename T>
        void onAddComponent(uint32_t entity, const T& component)
        {
            if (T::GetComponentID() == ScriptComponent::GetComponentID())
                m_EntitiesWithScript.push_back(entity);
        }

        template <typename T>
        void onRemoveComponent(uint32_t entity)
        {
            if (T::GetComponentID() == ScriptComponent::GetComponentID())
            {
                auto it = std::find(m_EntitiesWithScript.begin(), m_EntitiesWithScript.end(), entity);
                if (it != m_EntitiesWithScript.end())
                    m_EntitiesWithScript.erase(it);
            }
        }

    private:
        ECSManager m_ECS;
        GUID m_UUID;

        std::vector<uint32_t> m_EntitiesWithScript;

        ComponentView<TransformComponent, SpriteRenderer>* m_RenderView;
        ComponentView<TransformComponent, ParticleComponent>* m_ParticleView;
        ComponentView<TransformComponent, PointLight2D>* m_LightView;
        ComponentView<AnimatorComponent>* m_AnimatorView;
        ComponentStorage<ScriptComponent>* m_ScriptStorage;
        ComponentStorage<AnimatorComponent>* m_AnimatorStorage;

        std::string m_Name;
        SceneState m_State = SceneState::Edit;

        uint32_t m_SelectedEntity;
        uint32_t m_CameraEntity;

        std::vector<uint32_t> m_Entities;
        std::unordered_map<uint32_t, uint32_t> m_SceneGraphMap;

        uint32_t m_ViewportWidth;
        uint32_t m_ViewportHeight;
        

        Ref<Material> m_CameraMaterial;
        Ref<Texture2D> m_CameraTexture;
        Ref<SubTexture> m_CameraSubTexture;
        SpriteRenderer* m_CameraSprite;
         
        friend class SceneEntity;
        friend class Serializer;
        friend class ScriptEngine;
        friend class SceneHierarchyPanel;
    };
}