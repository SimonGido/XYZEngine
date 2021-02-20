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

#include "XYZ/Physics/DynamicTree.h"
#include "XYZ/Physics/PhysicsWorld.h"

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

        void OnPlay();
        void OnStop();
        void OnUpdate(Timestep ts);
        void OnRender();
        void OnRenderEditor(const EditorCamera& camera);

        SceneEntity GetEntity(uint32_t index);
        SceneEntity GetSelectedEntity();
        inline const std::vector<uint32_t>& GetEntities() const { return m_Entities; }

        inline SceneState GetState() const { return m_State; }
        inline const GUID& GetUUID() const { return m_UUID; }
        inline const std::string& GetName() const { return m_Name; }

    private:
        void showSelection(uint32_t entity);
        void showCamera(uint32_t entity);


    private:
        ECSManager m_ECS;
        GUID m_UUID;
        PhysicsWorld m_PhysicsWorld;

        std::vector<uint32_t> m_Entities;

        ComponentView<TransformComponent, SpriteRenderer>* m_RenderView;
        ComponentView<TransformComponent, ParticleComponent>* m_ParticleView;
        ComponentView<TransformComponent, PointLight2D>* m_LightView;
        ComponentView<TransformComponent, RigidBody2DComponent>* m_RigidBodyView;

        ComponentView<AnimatorComponent>* m_AnimatorView;
        ComponentStorage<ScriptComponent>* m_ScriptStorage;
        ComponentStorage<AnimatorComponent>* m_AnimatorStorage;

        std::string m_Name;
        SceneState m_State = SceneState::Edit;

        uint32_t m_SelectedEntity;
        uint32_t m_CameraEntity;


        uint32_t m_ViewportWidth;
        uint32_t m_ViewportHeight;

        Ref<Texture2D> m_CameraTexture;
        Ref<SubTexture> m_CameraSubTexture;
        Ref<Material> m_CameraMaterial;
        SpriteRenderer m_CameraRenderer;


        friend class SceneEntity;
        friend class Serializer;
        friend class ScriptEngine;
        friend class ScenePanel;
        friend class LuaEntity;
        friend class SceneHierarchyPanel;
    };
}