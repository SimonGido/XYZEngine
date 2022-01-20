#pragma once
#include "XYZ/Core/Ref/Ref.h"
#include "XYZ/Core/Ref/WeakRef.h"
#include "XYZ/Core/Timestep.h"

#include "XYZ/ECS/ECSManager.h"

#include "XYZ/Event/Event.h"
#include "XYZ/Renderer/Camera.h"

#include "XYZ/Physics/ContactListener.h"
#include "XYZ/Physics/PhysicsWorld2D.h"

#include "XYZ/Utils/DataStructures/ThreadPass.h"
#include "XYZ/Asset/Asset.h"

#include "SceneCamera.h"
#include "Components.h"

#include <box2d/box2d.h>

namespace XYZ {

    enum class SceneState
    {
        Play,
        Edit,
        Pause
    };


    class Renderer2D;
    class SceneRenderer;
    class SceneEntity;

    namespace Editor {
        class SceneHierarchyPanel;
    }
    class Scene : public Asset
    {
    public:
        Scene(const std::string& name);
        ~Scene();

        SceneEntity CreateEntity(const std::string& name, const GUID& guid);
        SceneEntity CreateEntity(const std::string& name, SceneEntity parent, const GUID& guid);
        void DestroyEntity(SceneEntity entity);
        void SetState(SceneState state) { m_State = state; }
        void SetViewportSize(uint32_t width, uint32_t height);
        void SetSelectedEntity(Entity entity) { m_SelectedEntity = entity; }

        void OnPlay();
        void OnStop();
        void OnUpdate(Timestep ts);
        void OnRender(Ref<SceneRenderer> sceneRenderer);
        void OnRenderEditor(Ref<SceneRenderer> sceneRenderer, const glm::mat4& viewProjection, const glm::mat4& view, const glm::vec3& camPos, Timestep ts);

        SceneEntity GetEntity(uint32_t index);
        SceneEntity GetEntityByName(const std::string& name);
        SceneEntity GetSceneEntity();
        SceneEntity GetSelectedEntity();

        ECSManager&       GetECS() { return m_ECS; }
        const ECSManager& GetECS() const { return m_ECS; }
        inline const std::vector<Entity>& GetEntities() const { return m_Entities; }

        inline SceneState GetState() const { return m_State; }
        inline const GUID& GetUUID() const { return m_UUID; }
        inline const std::string& GetName() const { return m_Name; }

        static AssetType GetStaticType() { return AssetType::Scene; }
    private:
        void onScriptComponentConstruct(Entity entity);
        void onScriptComponentDestruct(Entity entity);

        void updateHierarchy();
        void setupPhysics();

        void sortSpriteRenderers();

        struct SpriteRenderData
        {
            SpriteRenderer*     Renderer;
            TransformComponent* Transform;
            uint64_t			SortKey;
        };

        std::vector<SpriteRenderData> m_SpriteRenderData;

    private:
        PhysicsWorld2D      m_PhysicsWorld;
        ContactListener     m_ContactListener;
        SceneEntity*        m_PhysicsEntityBuffer;

        ECSManager          m_ECS;
        GUID                m_UUID;
        Entity              m_SceneEntity;
        std::vector<Entity> m_Entities;

        std::string m_Name;
        SceneState  m_State;

        Entity m_SelectedEntity;
        Entity m_CameraEntity;


        uint32_t m_ViewportWidth;
        uint32_t m_ViewportHeight;


        friend class SceneEntity;
        friend class SceneSerializer;
        friend class ScriptEngine;
        friend class LuaEntity;
        friend class Editor::SceneHierarchyPanel;
    };
}