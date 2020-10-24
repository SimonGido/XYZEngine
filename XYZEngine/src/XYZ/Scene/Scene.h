#pragma once
#include "XYZ/Core/Ref.h"
#include "XYZ/Core/Timestep.h"
#include "XYZ/ECS/ECSManager.h"
#include "XYZ/Event/Event.h"

#include "XYZ/Renderer/SpriteRenderer.h"
#include "XYZ/Renderer/Camera.h"

#include "EditorCamera.h"
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
    class Entity;
    class Scene : public RefCount,
                  public Serializable
    {
    public:
        Scene(const std::string& name);
        ~Scene();
    
        Entity CreateEntity(const std::string& name);

        void DestroyEntity(Entity entity);

        void SetParent(Entity parent, Entity child);

        void SetState(SceneState state) { m_State = state; }

        /** Triggers when the scene is attached */
        void OnAttach();

        /** Triggers when the scene is detached */
        void OnDetach();

        void OnEvent(Event& e);


        void OnUpdate(Timestep ts);

        void OnRender();
        void OnRenderEditor(const EditorCamera& camera);

        void SetViewportSize(uint32_t width, uint32_t height);

        SceneState GetState() const { return m_State; }

        Entity GetEntity(uint32_t index);

        inline const std::string& GetName() const { return m_Name; }

        inline const SceneCamera& GetMainCamera() const { return m_MainCamera->Camera; }
 
        ECSManager& GetECS() { return m_ECS; }

        void SetSelectedEntity(uint32_t entity) { m_SelectedEntity = entity; }

    private:
        void showSelection(uint32_t entity);

    private:
        ECSManager m_ECS;
        ComponentGroup<TransformComponent, SpriteRenderer>* m_RenderGroup = nullptr;
        ComponentGroup<AnimatorComponent>* m_AnimateGroup = nullptr;
        ComponentGroup<NativeScriptComponent>* m_ScriptGroup = nullptr;

        std::string m_Name;
        SceneState m_State = SceneState::Edit;

        uint32_t m_SelectedEntity;
        uint32_t m_MainCameraEntity;
        CameraComponent* m_MainCamera;
        TransformComponent* m_MainCameraTransform;

        std::vector<uint32_t> m_Entities;
        std::unordered_map<uint32_t, uint32_t> m_SceneGraphMap;

     
        friend class Entity;
        friend class Asset<Scene>;
        friend class SceneHierarchyPanel;
    };
}