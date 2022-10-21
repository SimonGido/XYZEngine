#pragma once
#include "XYZ/Core/Ref/Ref.h"
#include "XYZ/Core/Ref/WeakRef.h"
#include "XYZ/Core/Timestep.h"

#include "XYZ/Event/Event.h"
#include "XYZ/Renderer/Camera.h"

#include "XYZ/Physics/ContactListener.h"
#include "XYZ/Physics/PhysicsWorld2D.h"

#include "XYZ/Utils/DataStructures/ThreadPass.h"
#include "XYZ/Asset/Asset.h"
#include "XYZ/Asset/Animation/AnimationController.h"
#include "XYZ/Asset/Renderer/MeshSource.h"
#include "XYZ/Renderer/Mesh.h"
#include "XYZ/Particle/GPU/ParticleSystemGPU.h"

#include "SceneCamera.h"

#include <entt/entt.hpp>

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


    struct PointLight3D
    {
        glm::vec3 Position = { 0.0f, 0.0f, 0.0f };
        float	  Multiplier = 0.0f;
        glm::vec3 Radiance = { 0.0f, 0.0f, 0.0f };
        float	  MinRadius = 0.001f;
        float	  Radius = 25.0f;
        float	  Falloff = 1.f;
        float	  SourceSize = 0.1f;
        bool	  CastsShadows = true;
        char	  Padding[3]{ 0, 0, 0 };
    };
    
    struct LightEnvironment
    {
        std::vector<PointLight3D> PointLights3D;
    };

    class Scene : public Asset
    {
    public:
        Scene(const std::string& name, const GUID& guid = GUID());
        ~Scene();

        SceneEntity CreateEntity(const std::string& name, const GUID& guid = GUID());
        SceneEntity CreateEntity(const std::string& name, SceneEntity parent, const GUID& guid = GUID());
   

        void DestroyEntity(SceneEntity entity);
        void SetState(SceneState state) { m_State = state; }
        void SetViewportSize(uint32_t width, uint32_t height);
        void SetSelectedEntity(entt::entity ent) { m_SelectedEntity = ent; }

        void OnPlay();
        void OnStop();
        void OnUpdate(Timestep ts);
        void OnRender(Ref<SceneRenderer> sceneRenderer);
        void OnUpdateEditor(Timestep ts);
        void OnRenderEditor(Ref<SceneRenderer> sceneRenderer, const glm::mat4& viewProjection, const glm::mat4& view, const glm::vec3& camPos);

        
        void OnImGuiRender();

        SceneEntity GetEntityByName(const std::string& name);
        SceneEntity GetEntityByGUID(const GUID& guid);
        SceneEntity GetSceneEntity();
        SceneEntity GetSelectedEntity();

        entt::registry&       GetRegistry()       { return m_Registry; }
        const entt::registry& GetRegistry() const { return m_Registry; }

        inline SceneState  GetState() const { return m_State; }
        inline const GUID& GetUUID() const { return m_UUID; }
        inline const std::string& GetName() const { return m_Name; }

        static AssetType GetStaticType() { return AssetType::Scene; }
    private:
        void onScriptComponentConstruct(entt::registry& reg, entt::entity ent);
        void onScriptComponentDestruct(entt::registry& reg, entt::entity ent);

        void updateScripts(Timestep ts);
        void updateHierarchy();      
        void updateHierarchyAsync();
        void updateSubHierarchy(entt::entity parent);


        void updateAnimationView(Timestep ts);
        void updateAnimationViewAsync(Timestep ts);

        void updateParticleView(Timestep ts);
        void updateRigidBody2DView();

        void updateParticleGPUView(Timestep ts);

        void setupPhysics();
        void setupLightEnvironment();

        void submitParticleGPUView();

    private:
        PhysicsWorld2D      m_PhysicsWorld;
        ContactListener     m_ContactListener;
        SceneEntity*        m_PhysicsEntityBuffer;
        LightEnvironment    m_LightEnvironment;

        entt::registry      m_Registry;
        GUID                m_UUID;
        entt::entity        m_SceneEntity;

        std::string m_Name;
        SceneState  m_State;

        entt::entity m_SelectedEntity;
        entt::entity m_CameraEntity;


        uint32_t m_ViewportWidth;
        uint32_t m_ViewportHeight;

        std::shared_mutex m_ScriptMutex;


        bool m_UpdateAnimationAsync = false;
        bool m_UpdateHierarchyAsync = false;

        friend SceneRenderer;
        friend class SceneIntersection;
        friend class SceneEntity;
        friend class SceneSerializer;
        friend class ScriptEngine;
        friend class LuaEntity;
        friend class Editor::SceneHierarchyPanel;


        void createParticleTest();

        // Indirect draw test //
        Ref<MaterialAsset>     m_IndirectCommandMaterial;
        Ref<ParticleSystemGPU> m_ParticleSystemGPU;

        Ref<MaterialAsset>	   m_ParticleMaterialGPU;
        Ref<MaterialInstance>  m_ParticleMaterialInstanceGPU;
        Ref<Mesh>			   m_ParticleCubeMesh;
    };
}