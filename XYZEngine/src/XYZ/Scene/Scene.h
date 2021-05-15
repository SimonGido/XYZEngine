#pragma once
#include "XYZ/Core/Ref.h"
#include "XYZ/Core/Timestep.h"
#include "XYZ/ECS/ECSManager.h"

#include "XYZ/Event/Event.h"
#include "XYZ/Renderer/Camera.h"

#include "XYZ/Editor/EditorCamera.h"
#include "SceneCamera.h"
#include "Components.h"
#include "EditorComponents.h"

#include "XYZ/Asset/Asset.h"

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
        void OnRender();
        void OnRenderEditor(const Editor::EditorCamera& camera);

        SceneEntity GetEntity(uint32_t index);
        SceneEntity GetEntityByName(const std::string& name);
        SceneEntity GetSelectedEntity();
        ECSManager& GetECS() {return m_ECS;}
        inline const std::vector<Entity>& GetEntities() const { return m_Entities; }

        inline SceneState GetState() const { return m_State; }
        inline const GUID& GetUUID() const { return m_UUID; }
        inline const std::string& GetName() const { return m_Name; }


        static Ref<Scene> ActiveScene;
    private:
        void showSelection(uint32_t entity);
        void showCamera(uint32_t entity);
        void updateHierarchy();


    private:
        ECSManager m_ECS;
        GUID m_UUID;
        PhysicsWorld m_PhysicsWorld;
        Entity m_SceneEntity;
        std::vector<Entity> m_Entities;

        std::string m_Name;
        SceneState m_State = SceneState::Edit;

        Entity m_SelectedEntity;
        Entity m_CameraEntity;


        uint32_t m_ViewportWidth;
        uint32_t m_ViewportHeight;

        Ref<Texture2D> m_CameraTexture;
        Ref<SubTexture> m_CameraSubTexture;
        Ref<Material> m_CameraMaterial;
        SpriteRenderer m_CameraRenderer;


        

        friend class SceneEntity;
        friend class SceneSerializer;
        friend class ScriptEngine;
        friend class ScenePanel;
        friend class LuaEntity;
        friend class Editor::SceneHierarchyPanel;
    };
}